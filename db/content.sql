/*
    This file contants inital content for any tables.

    The table must be first defined in schema.sql before any content
    is added!
*/
insert into consent_type (id, shortname, description, enabled, protect, privacypref)
    values (1, 'ENROLL', 'General terms-of-use for this BOINC project.', 0, 1, 0),
    (2, 'STATSEXPORT', 'Do you consent to exporting your data to BOINC statistics aggregation Web sites?', 0, 1, 1);
