/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020-2023 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef PCB_IO_FABMASTER_H_
#define FABMASTER_PLUGIN_H_


#include "import_fabmaster.h"

#include <pcb_io/pcb_io.h>
#include <pcb_io/pcb_io_mgr.h>

class PCB_IO_FABMASTER : public PCB_IO
{
public:
    PLUGIN_FILE_DESC GetBoardFileDesc() const override;

    BOARD* LoadBoard( const wxString& aFileName, BOARD* aAppendToMe,
                      const STRING_UTF8_MAP* aProperties = nullptr, PROJECT* aProject = nullptr,
                      PROGRESS_REPORTER* aProgressReporter = nullptr ) override;

    long long GetLibraryTimestamp( const wxString& aLibraryPath ) const override
    {
        // No support for libraries....
        return 0;
    }

    PCB_IO_FABMASTER();
    ~PCB_IO_FABMASTER();

private:
    FABMASTER              m_fabmaster;
};

#endif    // PCB_IO_FABMASTER_H_
