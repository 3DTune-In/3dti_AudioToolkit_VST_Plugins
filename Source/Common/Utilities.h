//
//  Utilities.h
//  3D Tune-In Hearing Aid Simulator - Shared Code
//
//  Created by Ragnar Hrafnkelsson on 30/05/2020.
//

#ifndef Utilities_h
#define Utilities_h

#include <JuceHeader.h>

static forcedinline String abbreviatedFrequency (float frequency)
{
    return String (frequency).replace ("000", "k");
}

#endif /* Utilities_h */
