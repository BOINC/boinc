alter table bolt_course
    add unique(name);

alter table bolt_enrollment
    add unique(user_id, course_id);

alter table bolt_view
    add index bv_cs(course_id, start_time);
