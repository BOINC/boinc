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
package edu.berkeley.boinc.rpc;

import com.google.common.testing.EqualsTester;

import org.junit.Test;

public class ResultTest {
    @Test
    public void testEqualsAndHashCode() {
        final Result result = new Result();
        result.name = "Name";

        final Result result1 = new Result();
        result1.wu_name = "Work Unit";

        final Result result2 = new Result();
        result2.project_url = "Project URL";

        final Result result3 = new Result();
        result3.version_num = 1;

        final Result result4 = new Result();
        result4.plan_class = "Plan class";

        final Result result5 = new Result();
        result5.report_deadline = 1;

        final Result result6 = new Result();
        result6.received_time = 1;

        final Result result7 = new Result();
        result7.final_cpu_time = 1;

        final Result result8 = new Result();
        result8.final_elapsed_time = 1;

        final Result result9 = new Result();
        result9.state = 1;

        final Result result10 = new Result();
        result10.scheduler_state = 1;

        final Result result11 = new Result();
        result11.exit_status = 1;

        final Result result12 = new Result();
        result12.signal = 1;

        final Result result13 = new Result();
        result13.stderr_out = "Output";

        final Result result14 = new Result();
        result14.active_task_state = 1;

        final Result result15 = new Result();
        result15.app_version_num = 1;

        final Result result16 = new Result();
        result16.slot = 1;

        final Result result17 = new Result();
        result17.pid = 1;

        final Result result18 = new Result();
        result18.checkpoint_cpu_time = 1;

        final Result result19 = new Result();
        result19.current_cpu_time = 1;

        final Result result20 = new Result();
        result20.fraction_done = 1;

        final Result result21 = new Result();
        result21.elapsed_time = 1;

        final Result result22 = new Result();
        result22.swap_size = 1;

        final Result result23 = new Result();
        result23.working_set_size_smoothed = 1;

        new EqualsTester().addEqualityGroup(new Result(), new Result())
                          .addEqualityGroup(result)
                          .addEqualityGroup(result1)
                          .addEqualityGroup(result2)
                          .addEqualityGroup(result3)
                          .addEqualityGroup(result4)
                          .addEqualityGroup(result5)
                          .addEqualityGroup(result6)
                          .addEqualityGroup(result7)
                          .addEqualityGroup(result8)
                          .addEqualityGroup(result9)
                          .addEqualityGroup(result10)
                          .addEqualityGroup(result11)
                          .addEqualityGroup(result12)
                          .addEqualityGroup(result13)
                          .addEqualityGroup(result14)
                          .addEqualityGroup(result15)
                          .addEqualityGroup(result16)
                          .addEqualityGroup(result17)
                          .addEqualityGroup(result18)
                          .addEqualityGroup(result19)
                          .addEqualityGroup(result20)
                          .addEqualityGroup(result21)
                          .addEqualityGroup(result22)
                          .addEqualityGroup(result23)
                          .testEquals();
    }
}
