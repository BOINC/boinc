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
package edu.berkeley.boinc.rpc

import com.google.common.testing.EqualsTester
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test
import org.junit.jupiter.api.extension.ExtensionContext
import org.junit.jupiter.params.ParameterizedTest
import org.junit.jupiter.params.provider.Arguments
import org.junit.jupiter.params.provider.ArgumentsProvider
import org.junit.jupiter.params.provider.ArgumentsSource
import java.util.stream.Stream

private class AppArgumentsProvider : ArgumentsProvider {
    override fun provideArguments(context: ExtensionContext?): Stream<out Arguments> {
        return Stream.of(
                Arguments.of(App("Name", null)),
                Arguments.of(App("Name", "")),
                Arguments.of(App("Name", "User-friendly name"))
        )
    }
}

class AppTest {
    @ParameterizedTest
    @ArgumentsSource(AppArgumentsProvider::class)
    fun `Test displayName property`(app: App) {
        if (app.userFriendlyName.isNullOrEmpty())
            Assertions.assertEquals(app.displayName, app.name)
        else
            Assertions.assertEquals(app.displayName, app.userFriendlyName)
    }

    @Test
    fun `Test equals() and hashCode()`() {
        EqualsTester().addEqualityGroup(App("Name"), App("NAME"))
                .addEqualityGroup(App(userFriendlyName = "User-friendly name"),
                        App(userFriendlyName = "USER-FRIENDLY NAME"))
                .testEquals()
    }
}
