package edu.berkeley.boinc.adapter

import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.FragmentActivity
import androidx.fragment.app.testing.launchFragmentInContainer
import edu.berkeley.boinc.ProjectsFragment
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.Project
import org.apache.commons.lang3.builder.EqualsBuilder
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mockito.`when`
import org.mockito.Mockito.mock
import org.robolectric.RobolectricTestRunner


@RunWith(RobolectricTestRunner::class)
class ProjectsListAdapterTest {
    private lateinit var projectsListAdapter: ProjectsListAdapter
    private lateinit var fragActivity: FragmentActivity
    private lateinit var projectsList: List<ProjectsFragment.ProjectsListData>
    private lateinit var viewGroup: ViewGroup
    private lateinit var view: View

    @Before
    fun setUp() {
        val projectsFragment = ProjectsFragment()
        viewGroup = mock(ViewGroup::class.java)
        view = mock(View::class.java)
        val scenario = launchFragmentInContainer<ProjectsFragment>()
        scenario.onFragment {
            fragActivity = it.activity!!
        }
        projectsList = listOf(
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 1", diskUsage = 5000000.234),
                null,
                null
            ),
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 2", diskUsage = 5000000.345),
                null,
                null
            ),
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 3", diskUsage = 5000000.456),
                null,
                null
            ),
        )
        projectsListAdapter = ProjectsListAdapter(fragActivity, fragActivity.findViewById(R.id.projects_list), R.id.projects_list, projectsList)
    }

    @Test
    fun `Check that entry count equal passed count`() {
        Assert.assertEquals(3, projectsListAdapter.count)
    }

    @Test
    fun getItemTest() {
        Assert.assertTrue(EqualsBuilder.reflectionEquals(projectsList[1], projectsListAdapter.getItem(1)))
    }

    @Test
    fun getItemIdTest() {
        Assert.assertEquals(2L, projectsListAdapter.getItemId(2))
    }

    @Test
    fun getDiskUsageTest() {
        Assert.assertEquals("4.77", projectsListAdapter.getDiskUsage(1))
    }

    @Test
    fun `Check content and order of entries`() {
        Assert.assertEquals("Project 1", projectsListAdapter.getItem(0).project?.projectName
        )
        Assert.assertEquals(0, projectsListAdapter.getItemId(0))
        Assert.assertEquals("Project 2", projectsListAdapter.getItem(1).project?.projectName
        )
        Assert.assertEquals(1, projectsListAdapter.getItemId(1))
        Assert.assertEquals("Project 3", projectsListAdapter.getItem(2).project?.projectName
        )
        Assert.assertEquals(2, projectsListAdapter.getItemId(2))
    }
}