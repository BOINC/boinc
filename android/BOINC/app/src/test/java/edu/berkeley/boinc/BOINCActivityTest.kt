/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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

import androidx.test.espresso.Espresso.onView
import androidx.test.espresso.action.ViewActions.click
import androidx.test.espresso.assertion.ViewAssertions.matches
import androidx.test.espresso.matcher.ViewMatchers.*
import androidx.test.ext.junit.rules.ActivityScenarioRule
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.filters.LargeTest
import org.hamcrest.Matchers.allOf
import org.hamcrest.core.IsInstanceOf
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

// Disable BOINC notifications (or just the main channel on Oreo and higher) before running this test, as
// the client notification can cover the hamburger button during test execution and cause the test to fail.
@LargeTest
@RunWith(AndroidJUnit4::class)
class BOINCActivityTest {
    @Rule
    @JvmField
    var mActivityScenarioRule = ActivityScenarioRule(BOINCActivity::class.java)

    @Test
    fun boincActivityTest() {
        val textView = onView(
                allOf(withText(R.string.tab_tasks),
                        withParent(allOf(withId(R.id.action_bar),
                                withParent(withId(R.id.action_bar_container)))),
                        isDisplayed()))
        textView.check(matches(withText(R.string.tab_tasks)))

        val appCompatImageButton = onView(
                allOf(withContentDescription("BOINC"),
                        childAtPosition(
                                allOf(withId(R.id.action_bar),
                                        childAtPosition(
                                                withId(R.id.action_bar_container),
                                                0)),
                                1),
                        isDisplayed()))
        appCompatImageButton.perform(click())

        val viewInteraction = onView(allOf(withText(R.string.tab_notices), isDisplayed()))
        viewInteraction.perform(click())

        val textView2 = onView(
                allOf(withText(R.string.tab_notices),
                        withParent(allOf(withId(R.id.action_bar),
                                withParent(withId(R.id.action_bar_container)))),
                        isDisplayed()))
        textView2.check(matches(withText(R.string.tab_notices)))

        appCompatImageButton.perform(click())

        val viewInteraction2 = onView(allOf(withText(R.string.tab_projects), isDisplayed()))
        viewInteraction2.perform(click())

        val textView3 = onView(
                allOf(withText(R.string.tab_projects),
                        withParent(allOf(withId(R.id.action_bar),
                                withParent(withId(R.id.action_bar_container)))),
                        isDisplayed()))
        textView3.check(matches(withText(R.string.tab_projects)))

        appCompatImageButton.perform(click())

        val viewInteraction3 = onView(allOf(withText(R.string.tab_preferences), isDisplayed()))
        viewInteraction3.perform(click())

        val textView4 = onView(
                allOf(withText(R.string.tab_preferences),
                        withParent(allOf(withId(R.id.action_bar),
                                withParent(withId(R.id.action_bar_container)))),
                        isDisplayed()))
        textView4.check(matches(withText(R.string.tab_preferences)))

        appCompatImageButton.perform(click())

        val viewInteraction4 = onView(allOf(withText(R.string.menu_about), isDisplayed()))
        viewInteraction4.perform(click())

        val textView5 = onView(
                allOf(withId(R.id.title), withText(R.string.menu_about),
                        withParent(withParent(withId(android.R.id.content))),
                        isDisplayed()))
        textView5.check(matches(withText(R.string.menu_about)))

        val appCompatButton = onView(
                allOf(withId(R.id.returnB), withText(R.string.about_button),
                        childAtPosition(
                                childAtPosition(
                                        withId(android.R.id.content),
                                        0),
                                6),
                        isDisplayed()))
        appCompatButton.perform(click())

        appCompatImageButton.perform(click())

        val viewInteraction5 = onView(allOf(withText(R.string.menu_eventlog), isDisplayed()))
        viewInteraction5.perform(click())

        val textView6 = onView(
                allOf(withText(R.string.menu_eventlog),
                        withParent(allOf(withId(R.id.action_bar),
                                withParent(withId(R.id.action_bar_container)))),
                        isDisplayed()))
        textView6.check(matches(withText(R.string.menu_eventlog)))

        val textView7 = onView(
                allOf(withText(R.string.eventlog_client_header),
                        withParent(allOf(withContentDescription(R.string.eventlog_client_header),
                                withParent(IsInstanceOf.instanceOf(android.widget.LinearLayout::class.java)))),
                        isDisplayed()))
        textView7.check(matches(withText(R.string.eventlog_client_header)))

        val textView8 = onView(
                allOf(withText(R.string.eventlog_gui_header),
                        withParent(allOf(withContentDescription(R.string.eventlog_gui_header),
                                withParent(IsInstanceOf.instanceOf(android.widget.LinearLayout::class.java)))),
                        isDisplayed()))
        textView8.check(matches(withText(R.string.eventlog_gui_header)))

        val appCompatImageButton2 = onView(
                allOf(withContentDescription("Navigate up"),
                        childAtPosition(
                                allOf(withId(R.id.action_bar),
                                        childAtPosition(
                                                withId(R.id.action_bar_container),
                                                0)),
                                1),
                        isDisplayed()))
        appCompatImageButton2.perform(click())

        val textView9 = onView(
                allOf(withText(R.string.tab_tasks),
                        withParent(allOf(withId(R.id.action_bar),
                                withParent(withId(R.id.action_bar_container)))),
                        isDisplayed()))
        textView9.check(matches(withText(R.string.tab_tasks)))
    }
}
