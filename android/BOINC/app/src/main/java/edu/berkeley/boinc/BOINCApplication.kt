/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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
package edu.berkeley.boinc

import androidx.multidex.MultiDexApplication
import edu.berkeley.boinc.di.AppComponent
import edu.berkeley.boinc.di.DaggerAppComponent
import edu.berkeley.boinc.utils.applyTheme

open class BOINCApplication : MultiDexApplication() {
    override fun onCreate() {
        super.onCreate()
        applyTheme(this)
    }

    val appComponent: AppComponent by lazy {
        DaggerAppComponent.factory().create(applicationContext)
    }

    // Override in tests.
    open fun initializeComponent() = DaggerAppComponent.factory().create(applicationContext)
}
