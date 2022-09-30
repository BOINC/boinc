package edu.berkeley.boinc.adapter

import androidx.fragment.app.FragmentActivity
import androidx.fragment.app.testing.launchFragmentInContainer
import edu.berkeley.boinc.R
import edu.berkeley.boinc.ProjectsFragment
import edu.berkeley.boinc.rpc.Project
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class ProjectsListAdapterTest {
    private lateinit var projectsListAdapter: ProjectsListAdapter
    private lateinit var fragActivity: FragmentActivity


    @Before
    fun setUp() {
        val projectsFragment = ProjectsFragment()
        val scenario = launchFragmentInContainer<ProjectsFragment>()
        scenario.onFragment {
            fragActivity = it.activity!!
        }
        val projectsList = listOf(
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 1"),
                null,
                null
            ),
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 2"),
                null,
                null
            ),
            projectsFragment.ProjectsListData(
                Project(projectName = "Project 3"),
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