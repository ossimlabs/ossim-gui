//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Place for common stuff.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimGuiCommon_HEADER
#define ossimGuiCommon_HEADER 1

#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageHandler.h>

#include <vector>

// Forward class declarations:
class QButtonGroup;

namespace ossimGui
{
   typedef std::vector<ossimRefPtr<ossimImageHandler> > HandlerList;
   

} // End: namespace ossimGui

#endif /* #ifndef ossimGuiCommon_HEADER */
