/*
    This file contains initial content for tables.

    The table must be first defined in schema.sql before any content
    is added!
*/
insert into consent_type (shortname, description, enabled, project_specific, privacypref)
    values ('ENROLL', 'General terms-of-use for this BOINC project.', 0, 0, 0),
    ('STATSEXPORT', 'Do you consent to exporting your data to BOINC statistics aggregation Web sites?', 0, 0, 1);
