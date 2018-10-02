/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <cfenv>
#include <cmath>
#include <string>
#include <sstream>

#include "SafeMaths.hpp"

Fn::SafeMaths::func Fn::SafeMaths::getFunction( IOsptr sample_input )
{
    // get function.. nothing to test
    func _func = _fn->getFunction( sample_input );
    
    // create and return the lambda
    return [_func, this](IOsptr input)->IOsptr {
        std::fenv_t envp;
        
        // record and then clear existing fenv
        int notsuccess = std::feholdexcept( &envp ) ;
        if (notsuccess)
            throw std::runtime_error(_pyfnerrorheader + "Unknown error. Please contact developers.");
        
        // perform func
        IOsptr _output = _func(input);
        // check for errors
        if( std::fetestexcept( FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW ) )
        {
            
            std::stringstream ss;
            ss << "Floating point exception(s) encountered while evaluating SafeMaths argument function:\n";
            if( std::fetestexcept(FE_DIVBYZERO) )
                ss << "   Divide by zero";
            if( std::fetestexcept(FE_INVALID)   )
                ss << "   Invalid domain";
            if( std::fetestexcept(FE_OVERFLOW)  )
                ss << "   Value overflow";
            if( std::fetestexcept(FE_UNDERFLOW) )
                ss << "   Value underflow";
            
            // restore original env before throwing
            notsuccess = std::feupdateenv( &envp );
            if (notsuccess)
                throw std::runtime_error(_pyfnerrorheader + "Unknown error. Please contact developers.");
            throw std::runtime_error(_pyfnerrorheader + ss.str());
        }
        
        // ok, we got this far, restore fenv and continue
        notsuccess = std::feupdateenv( &envp );
        if (notsuccess)
            throw std::runtime_error(_pyfnerrorheader + "Unknown error. Please contact developers.");
        
        return _output;
    };
    
}


