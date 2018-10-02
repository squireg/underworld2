##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
'''
This module implements some high level timing operations for Underworld,
allowing users to determine how walltime is divided between different
Underworld API calls.  Note that this module *only* records timing
for Underworld API calls, and has no way of knowing how much time has
been spent elsewhere (such as `numpy`, `scipy` etc). The total runtime
is also recorded which gives users an indication of how much time
is spent outside Underworld.

Timing routines enabled by this module should introduce neglibile
computational overhead, and the user should feel confident enabling
them at all times without concern for efficiency implications.

Only the root process records timing information.

Note that Underworld API methods are automatically instrumented
with timing routines at `import` time, though the user must activate
storing timing data via a call to `timing.start()`. To completely
disable timing instrumentation, the user should set the
`UW_DISABLE_TIMING` environment variable.

Example
-------
>>> import underworld as uw
>>> uw.timing.start()
>>> someMesh = uw.mesh.FeMesh_Cartesian()
>>> with someMesh.deform_mesh():
...     someMesh.data[0] = [0.1,0.1]
>>> uw.timing.stop()
>>> # uw.print_table()   # This will print the data.
>>>                      # Commented out as not doctest friendly.

'''

from collections import defaultdict as _dd
import time as _time
import inspect as _inspect
import underworld as _uw
import os as _os

timing = False

def start():
    """
    Call this function to start recording timing data.
    """
    depth=0
    global timing
    global _maxdepth
    global _currentDepth
    _currentDepth = 0
    if _uw.rank() == 0 and ("UW_DISABLE_TIMING" not in _os.environ):
        timing = True
    _maxdepth = depth + 1
    global _starttime
    _starttime = _time.time()

def stop():
    """
    Call this function to stop recording timing data.
    Note that this is automatically called when
    `print_table()` is called.
    """
    global _endtime
    _endtime = _time.time()
    global timing
    if timing:
        timing = False

def reset():
    """
    Reset timing data. Note that this function calls
    `stop()`, and the user must call `start()` to resume
    recording timing data.
    """
    stop()
    global _hit_count
    _hit_count = _dd(lambda: [0,0.])

# go ahead and reset
reset()

def get_data(group_by="line_routine"):
    """
    Returns dict with timing data.
    
    Parameters
    ----------
    group_by: str
        Reported timing data is grouped according to the following options:
        "line"        : Calling line of code.
        "routine"     : Class routine.
        "line_routine": Line&routine form an individual timing group.
    """
    if _uw.rank() != 0:
        return

    # function to convert key into useful text
    def linefunc( key ):
        if key[1].startswith("<ipython-input-"):
            spltstr = key[1].split("-")
            no_cell = int(spltstr[2])
            no_line = key[2]
            return "Cell: {:>3}  Line:{:>3}".format(no_cell,no_line)
        else:
            return "{}:{:>5}".format(key[1],key[2])

    if group_by == "line":
        keyfunc = linefunc
    elif group_by == "routine":
        keyfunc = lambda key : key[0]
    elif group_by == "line_routine":
        keyfunc = lambda key : "{}   {}".format(linefunc(key),key[0])
    else:
        raise ValueError("'group_by' parameter should specify 'line', 'routine' 'line_routine'" )

    # regroup data
    regrouped_dict = _dd(lambda: [0,0.])
    for key, value in _hit_count.iteritems():
        data = regrouped_dict[keyfunc(key)]
        data[0] += value[0]
        data[1] += value[1]

    return regrouped_dict

def print_table(group_by="line_routine", sort_by="total", display_fraction=0.95, float_precision=".3f", output_file=None, **kwargs ):
    """
    Print timing results to stdout or to a provided file. Call this function
    stops timing.
    
    Parameters
    ----------
    group_by: str
        See `get_data()` function
    sort_by: str
        Data is sorted according to:
        "total"   : Total time allocated to any group.
        "average" : Average time attributed to any group
    display_fraction: float
        Set this option to cull insignificant (short time) results.
    output_file: str
        File to record table to. If none provided, outputs to stdout.
    **kwargs
        Any extra kwargs are passed to `tabulate` module (if installed).
        This allows you to tweak the output format. Consule the `tabulate`
        module instructions for details.

    """
    stop()
    if _uw.rank() != 0:
        return

    regrouped_dict = get_data(group_by)
    
    # convert to list
    table_data = []
    for key, value in regrouped_dict.iteritems():
        row = [ key, value[0], value[1], value[1]/value[0] ]
        table_data.append(row)

    sort_col = { "total":2, "average":3 }
    if sort_by not in sort_col.keys():
        raise ValueError("'sort_by' parameter should specify one of {}".format(sort_col.keys()))
    table_data = sorted(table_data, key= lambda x: x[sort_col[sort_by]], reverse=True)

    # get tots time
    all_time = 0.
    for row in table_data:
        all_time+= row[2]

    # max sure columns withds accommodate titles
    row_title = [group_by, "hits", "tot_time", "av_time"]
    maxl = [0,0,0,0]
    for ii in range(0,4):
        maxl[ii] = max(maxl[ii],len(row_title[ii]))

    # get index for cutoff, and also column widths
    inc_time = 0.
    formatstr = "{0:"+float_precision+"}"
    for index,row in enumerate(table_data):
        inc_time += row[2]
        # calc string lengths of data
        maxl[0] = max(maxl[0],len(row[0]))
        maxl[1] = max(maxl[1],len("{}".format(row[1])))
        maxl[2] = max(maxl[2],len(formatstr.format(row[2])))
        maxl[3] = max(maxl[3],len(formatstr.format(row[3])))
        if inc_time/all_time > display_fraction:
            stop_row = index
            break


    # add a space between columns
    for ii in range(0,len(maxl)):
        maxl[ii] += 1

    footerrow = [[ None,                     None, None, None],
                 [ "Total Time (UW2 API) :", None,            all_time, None],
                 [ "Total Time (Runtime) :", None, _endtime-_starttime, None]]

    try:
        from tabulate import tabulate
        have_tab = True
    except:
        have_tab = False

    try:  # try using tabulate
        if False: #_uw._run_from_ipython():
            from IPython.display import HTML, display
            display(HTML(tabulate( table_data[0:stop_row]+footerrow, row_title, tablefmt='html',floatfmt=".3f", **kwargs )))
        else:
            tabstr = tabuflate(table_data[0:stop_row]+footerrow,row_title,floatfmt=".3f", **kwargs)
            tabstr += "\n"
    except: # othersise use homebake.. our hipster snowflakes friends (and colleague) are not forgotten!
        # add header
        tabstr = "{}".format(row_title[0]).ljust(maxl[0]) + \
                 "{}".format(row_title[1]).rjust(maxl[1]) + \
                 "{}".format(row_title[2]).rjust(maxl[2]) + \
                 "{}".format(row_title[3]).rjust(maxl[3]) + "\n"
        # add margin line
        tabstr += "-" * (maxl[0] -1) + "  "
        tabstr += "-" * (maxl[1] -1) + " "
        tabstr += "-" * (maxl[2] -1) + " "
        tabstr += "-" * (maxl[3] -1) + "\n"
        # add data colums
        for row in table_data[0:stop_row]:
            tabstr +="{}".format(row[0]).ljust(maxl[0]) + \
                     "{}".format(row[1]).rjust(maxl[1]) + \
                     formatstr.format(row[2]).rjust(maxl[2]) + \
                     formatstr.format(row[3]).rjust(maxl[3]) + "\n"
        tabstr += "\n"
        for row in footerrow[1:]:
            tabstr +="{}".format(row[0]).ljust(maxl[0]) + \
                     "".rjust(maxl[1]) + \
                     formatstr.format(row[2]).rjust(maxl[2]) + \
                     "".rjust(maxl[3]) + "\n"
    if output_file:
        with open(output_file, "w") as text_file:
            text_file.write(tabstr)
    else:
        print("")
        print(tabstr)

              
def _incrementDepth():
    """
    Manually increment depth counter.
    
    This is sometimes needed to let this module know that
    we are inside the Underworld API. In particular, for
    StgCompoundComponents, we manually record construction
    time and we need this to ensure that we do not record
    all the sub-calls within the constructors.
    """
    if timing:
        global _currentDepth
        _currentDepth += 1

def _decrementDepth():
    """
    Manually decrement depth counter.
    """
    if timing:
        global _currentDepth
        _currentDepth -= 1

def log_result( time, name ):
    """
    Allows the user to manually add entries to data.
    
    Parameters
    ----------
    time: float
        Time spent.
    name: str
        Name to record to dataset. Note that the current stack information
        is generated internally and recorded.
    """
    global _currentDepth
    if timing:
        if _currentDepth < _maxdepth:
            stk = _inspect.stack(0)
            data = _hit_count[(name,stk[2][1],stk[2][2])]
            data[0]+=1
            data[1]+=time

def _routine_timer_decorator(routine, class_name=None):
    """
    This decorator replaces any routine with the timed equivalent.
    """
    if routine.__name__ == "timed":  # parent class already added timing so skip
        return routine
    if class_name:
        recname = class_name+"."+routine.__name__+"()"
    else:
        recname = routine.__name__
    def timed(*args, **kwargs):
        global _currentDepth
        if timing:
            _currentDepth += 1
            if _currentDepth <= _maxdepth:
                stk = _inspect.stack(0)
                ts = _time.time()
                result = routine(*args, **kwargs)
                te = _time.time()
                data = _hit_count[(recname,stk[1][1],stk[1][2])]
                data[0]+= 1
                data[1]+= (te - ts)
                _currentDepth -= 1
                return result
            _currentDepth -= 1
        # if we get here, we're not timing, call routine / return result.
        return routine(*args, **kwargs)
    return timed

def _class_timer_decorator(cls):
    """
    This decorator walks the provided class decorating its
    methods with timed equivalents.
    """
    for attr in _inspect.getmembers(cls, _inspect.ismethod):
        if issubclass(cls, _uw._stgermain.StgCompoundComponent):  # metaclass captures constructor timing
            if attr[0] in ["__init__","__call__","__del__", "_setup"]:
                continue
        if attr[0] in ["__del__",]:
            continue # don't wrap destructors
#        if attr[0][0] == "_" and (issubclass(cls, _uw._stgermain.StgCompoundComponent)):  # metaclass captures constructor timing
#            continue
#        if attr[0][0] == "_" and (attr[0] != "__init__"):
#            continue
        timedroutine = _routine_timer_decorator(attr[1], cls.__name__)
        setattr(cls, attr[0], timedroutine)
    return cls


# keep a global list of done classes and modules
from sets import Set as _Set
_donemods = _Set()
_doneclss = _Set()

def _add_timing_to_mod(mod):
    """
    This function walks the provided module, seeking out classes
    to replace via the _class_timer_decorator.
    
    """
    if "UW_DISABLE_TIMING" in _os.environ:
        return

    import inspect
    moddir = _os.path.dirname(inspect.getfile(mod))
    lendir = len(moddir)

    # first gather info
    mods = []
    for guy in dir(mod):
        if guy[0] != "":  # don't grab private guys
            obj = getattr(mod,guy)
            if inspect.ismodule(obj): # add list of submodules
                if hasattr(obj, "__file__"):
                    objpath = _os.path.dirname(inspect.getfile(obj))
                    if objpath[0:lendir] != moddir:  # check if object is in current mod
                        continue
                else:
                    continue
                if obj not in _donemods:
                    _donemods.add(obj)
                    mods.append(obj)
            elif inspect.isclass(obj):
                # replace with timed version
                if obj not in _doneclss:
                    timed_obj = _class_timer_decorator(obj)
                    setattr(mod,guy,timed_obj)
                    _doneclss.add(obj)

    for mod in mods:
        # recurse into submodules
        _add_timing_to_mod(mod)

