/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
 *
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 */

package edu.berkeley.boinc.rpc;

import org.apache.commons.lang3.StringUtils;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

public class BaseParser extends DefaultHandler {
    protected StringBuilder mCurrentElement = new StringBuilder();
    protected boolean mElementStarted = false;

    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {
        super.characters(ch, start, length);
        if(mElementStarted) {
            // put it into StringBuilder
            if(mCurrentElement.length() == 0) {
                // still empty - trim leading whitespace characters and append
                mCurrentElement.append(StringUtils.stripStart(new String(ch), null));
            }
            else {
                // Non-empty - add everything
                mCurrentElement.append(ch, start, length);
            }
        }
    }

    protected void trimEnd() {
        // Trim trailing spaces
        String str = StringUtils.stripEnd(mCurrentElement.toString(), null);
        mCurrentElement.setLength(0);
        mCurrentElement.append(str);
    }
}
