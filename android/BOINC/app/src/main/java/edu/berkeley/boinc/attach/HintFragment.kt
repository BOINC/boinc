/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2022 University of California
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
package edu.berkeley.boinc.attach

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import edu.berkeley.boinc.R.layout
import edu.berkeley.boinc.utils.Logging.Category.GUI_VIEW
import edu.berkeley.boinc.utils.Logging.logVerbose

class HintFragment : Fragment() {
    private var type = 0
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        logVerbose(
            GUI_VIEW,
            "HintFragment onCreateView for hint type: $type"
        )
        var v: View? = null
        when (type) {
            HINT_TYPE_CONTRIBUTION -> v =
                inflater.inflate(layout.attach_project_hint_contribution_layout, container, false)
            HINT_TYPE_PROJECTWEBSITE -> v =
                inflater.inflate(layout.attach_project_hint_projectwebsite_layout, container, false)
            HINT_TYPE_PLATFORMS -> v =
                inflater.inflate(layout.attach_project_hint_platforms_layout, container, false)
        }
        return v
    }

    companion object {
        const val HINT_TYPE_CONTRIBUTION = 1
        const val HINT_TYPE_PROJECTWEBSITE = 2
        const val HINT_TYPE_PLATFORMS = 3
        fun newInstance(hintType: Int): HintFragment {
            val frag = HintFragment()
            frag.type = hintType
            return frag
        }
    }
}
