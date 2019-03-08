 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hcq.h"
#define INPUT_BUFFER_SIZE 256

/*
 * Return a pointer to the struct student with name stu_name
 * or NULL if no student with this name exists in the stu_list
 */
Student *find_student(Student *stu_list, char *student_name) {
    Student *student = stu_list;
    while(student != NULL) {
	if (strcmp(student->name, student_name)==0) {
	    return student;
	}
	student = student->next_overall;
    }
    return NULL;
}



/*   Return a pointer to the ta with name ta_name or NULL
 *   if no such TA exists in ta_list.
 */
Ta *find_ta(Ta *ta_list, char *ta_name) {
    Ta *cur = ta_list;
    while (cur != NULL) {
        if (strcmp(cur->name, ta_name)==0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}


/*  Return a pointer to the course with this code in the course list
 *  or NULL if there is no course in the list with this code.
 */
Course *find_course(Course *courses, int num_courses, char *course_code) {
    for(int i = 0; i < num_courses;i++) {
        if(courses[i].code == NULL) {
            return NULL;
        }
        if(strcmp(courses[i].code, course_code)==0) {
            return &courses[i];
        }
    }
    return NULL;
}


/* Add a student to the queue with student_name and a question about course_code.
 * if a student with this name already has a question in the queue (for any
   course), return 1 and do not create the student.
 * If course_code does not exist in the list, return 2 and do not create
 * the student struct.
 * For the purposes of this assignment, don't check anything about the
 * uniqueness of the name.
 */
int add_student(Student **stu_list_ptr, char *student_name, char *course_code,
    Course *course_array, int num_courses) {
    // check for existing student
    Student *existing_student = find_student(*stu_list_ptr, student_name);
    if (existing_student != NULL) {
        return 1;
    }

    //find course
    Course *course = find_course(course_array, num_courses, course_code);
    if (course == NULL) {
        return 2;
    }
    //Creating student
    Student *new_student = malloc(sizeof(Student));
    new_student->name = malloc(strlen(student_name)+1);
    strcpy(new_student->name, student_name);
    new_student->course = course;
    new_student->arrival_time = malloc(sizeof(time_t));
    time_t *now = malloc(sizeof(time_t));
    time(*(&now));
    new_student->arrival_time = now;
    new_student->next_overall = NULL;

    //updating course pointer links
    if (course->head == NULL) {
        course->head = new_student;
    } else if (course->tail == NULL) {
        course->tail = new_student;
        course->head->next_course = new_student;
    } else {
        course->tail->next_course = new_student;
        course->tail = new_student;
    }

    if (*stu_list_ptr == NULL) {
        *stu_list_ptr = new_student;
    } else {
        Student *cur = *stu_list_ptr;
        while (cur->next_overall != NULL) {
            cur = cur->next_overall;
        }
        cur->next_overall = new_student;
    }
    return 0;
}


/* Student student_name has given up waiting and left the help centre
 * before being called by a Ta. Record the appropriate statistics, remove
 * the student from the queues and clean up any no-longer-needed memory.
 *
 * If there is no student by this name in the stu_list, return 1.
 */
int give_up_waiting(Student **stu_list_ptr, char *student_name) {
    Student *cur = *stu_list_ptr;
    if (cur == NULL) {
        return 1;
    } else if (strcmp(cur->name, student_name) == 0) {
        cur->course->wait_time += difftime(time(0), *(cur->arrival_time));
        cur->course->bailed++;
        cur->course->head = cur->next_course;
        *stu_list_ptr = cur->next_overall;
        free(cur->arrival_time);
        free(cur->name);
        free(cur);
        return 0;
    }
    while (cur->next_overall != NULL) {
        if (strcmp(cur->next_overall->name, student_name)==0) {
            cur->course->wait_time += difftime(time(0), *(cur->arrival_time));
            cur->course->bailed++;
            Student *stu_tofree = cur->next_overall;
            if (stu_tofree->course->head == stu_tofree) {
                stu_tofree->course->head = stu_tofree->next_course;
                cur->next_overall = cur->next_overall->next_overall;
                free(stu_tofree->arrival_time);
                free(stu_tofree->name);
                free(stu_tofree);
                return 0;
            } else {
                Student *course_stu = stu_tofree->course->head;
                while (course_stu->next_course != NULL) {
                    if (course_stu->next_course == stu_tofree) {
                            course_stu->next_course = stu_tofree->next_course;
                            if (stu_tofree->course->tail == stu_tofree) {
                                stu_tofree->course->tail = course_stu;
                            }
                            cur->next_overall = cur->next_overall->next_overall;
                            free(stu_tofree->arrival_time);
                            free(stu_tofree->name);
                            free(stu_tofree);
                            return 0;
                    }
                    course_stu = course_stu->next_course;
                }
            }

        }
        cur = cur->next_overall;
    }

    return 1;
}

/* Create and prepend Ta with ta_name to the head of ta_list.
 * For the purposes of this assignment, assume that ta_name is unique
 * to the help centre and don't check it.
 */
void add_ta(Ta **ta_list_ptr, char *ta_name) {
    // first create the new Ta struct and populate
    Ta *new_ta = malloc(sizeof(Ta));
    if (new_ta == NULL) {
       perror("malloc for TA");
       exit(1);
    }
    new_ta->name = malloc(strlen(ta_name)+1);
    if (new_ta->name  == NULL) {
       perror("malloc for TA name");
       exit(1);
    }
    strcpy(new_ta->name, ta_name);
    new_ta->current_student = NULL;

    // insert into front of list
    new_ta->next = *ta_list_ptr;
    *ta_list_ptr = new_ta;
}

/* The TA ta is done with their current student.
 * Calculate the stats (the times etc.) and then
 * free the memory for the student.
 * If the TA has no current student, do nothing.
 */
void release_current_student(Ta *ta) {
    if (ta->current_student != NULL) {
        ta->current_student->course->helped++;
        ta->current_student->course->help_time += difftime(time(0), *(ta->current_student->arrival_time));
        free(ta->current_student->arrival_time);
        free(ta->current_student->name);
        free(ta->current_student);
        ta->current_student = NULL;
    }
}

/* Remove this Ta from the ta_list and free the associated memory with
 * both the Ta we are removing and the current student (if any).
 * Return 0 on success or 1 if this ta_name is not found in the list
 */
int remove_ta(Ta **ta_list_ptr, char *ta_name) {
    Ta *head = *ta_list_ptr;
    if (head == NULL) {
        return 1;
    } else if (strcmp(head->name, ta_name) == 0) {
        // TA is at the head so special case
        *ta_list_ptr = head->next;
        release_current_student(head);
        // memory for the student has been freed. Now free memory for the TA.
        free(head->name);
        free(head);
        return 0;
    }
    while (head->next != NULL) {
        if (strcmp(head->next->name, ta_name) == 0) {
            Ta *ta_tofree = head->next;
            //  We have found the ta to remove, but before we do that
            //  we need to finish with the student and free the student.
            //  You need to complete this helper function
            release_current_student(ta_tofree);

            head->next = head->next->next;
            // memory for the student has been freed. Now free memory for the TA.
            free(ta_tofree->name);
            free(ta_tofree);
            return 0;
        }
        head = head->next;
    }
    // if we reach here, the ta_name was not in the list
    return 1;
}






/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the full queue.
 * If the queue is empty, then TA ta_name simply finishes with the student
 * they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 */
int take_next_overall(char *ta_name, Ta *ta_list, Student **stu_list_ptr) {
    Ta *ta = find_ta(ta_list, ta_name);
    if (ta == NULL) {
        return 1;
    }
    release_current_student(ta);

    if (*stu_list_ptr != NULL) {
        Student *cur_student = *stu_list_ptr;
        ta->current_student = cur_student;
        cur_student->course->wait_time += difftime(time(0), *(cur_student->arrival_time));
        cur_student->course->head = cur_student->next_course;
        if (cur_student == cur_student->course->tail) {
            cur_student->course->tail = NULL;
        }
        *stu_list_ptr = cur_student->next_overall;
        free(cur_student->arrival_time);

        time_t *now = malloc(sizeof(time_t));
        time(*(&now));
        cur_student->arrival_time = now;
    }
    return 0;
}



/* TA ta_name is finished with the student they are currently helping (if any)
 * and are assigned to the next student in the course with this course_code.
 * If no student is waiting for this course, then TA ta_name simply finishes
 * with the student they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 * If course is invalid return 2, but finish with any current student.
 */
int take_next_course(char *ta_name, Ta *ta_list, Student **stu_list_ptr, char *course_code, Course *courses, int num_courses) {
    Ta *ta = find_ta(ta_list, ta_name);
    if (ta == NULL) {
        return 1;
    }
    release_current_student(ta);
    Course *course = find_course(courses, num_courses, course_code);
    if (course == NULL) {
        return 2;
    }
    if (course->head != NULL) {
        Student *stu_tofree = course->head;
        ta->current_student = stu_tofree;
        if (*stu_list_ptr == stu_tofree) {
            *stu_list_ptr = stu_tofree->next_overall;
        } else {
            Student *cur_student = *stu_list_ptr;
            while (cur_student->next_overall != NULL && cur_student->next_overall != stu_tofree) {
                cur_student = cur_student->next_overall;
            }
            cur_student->next_overall = stu_tofree->next_overall;
        }
        stu_tofree->course->head = stu_tofree->next_course;
        if (stu_tofree == stu_tofree->course->tail) {
            stu_tofree->course->tail = NULL;
        }
        stu_tofree->course->wait_time += difftime(time(0), *(stu_tofree->arrival_time));
        free(stu_tofree->arrival_time);

        time_t *now = malloc(sizeof(time_t));
        time(*(&now));
        stu_tofree->arrival_time = now;
    }
    return 0;
}


/* For each course (in the same order as in the config file), print
 * the <course code>: <number of students waiting> "in queue\n" followed by
 * one line per student waiting with the format "\t%s\n" (tab name newline)
 * Uncomment and use the printf statements below. Only change the variable
 * names.
 */
void print_all_queues(Student *stu_list, Course *courses, int num_courses) {
    for(int i=0; i<num_courses;i++) {
        int students_waiting = 0;
        Student *student = courses[i].head;
        while (student != NULL) {
                students_waiting++;
                student = student->next_course;
            }
        printf("%s: %d in queue\n", courses[i].code, students_waiting);
        Student *course_stu = courses[i].head;
        while (course_stu != NULL) {
            printf("\t%s\n", course_stu->name);
            course_stu = course_stu->next_course;
        }
    }
}


/*
 * Print to stdout, a list of each TA, who they are serving at from what course
 * Uncomment and use the printf statements
 */
void print_currently_serving(Ta *ta_list) {
    Ta *cur = ta_list;
    if(cur == NULL) {
        printf("No TAs are in the help centre.\n");
    } else {
        while (cur != NULL) {
            if (cur->current_student == NULL) {
                printf("TA: %s has no student\n", cur->name);
            } else {
                printf("TA: %s is serving %s from %s\n", cur->name, cur->current_student->name, cur->current_student->course->code);
            }
            cur = cur->next;
        }
    }
}


/*  list all students in queue (for testing and debugging)
 *   maybe suggest it is useful for debugging but not included in marking?
 */
void print_full_queue(Student *stu_list) {

}

/* Prints statistics to stdout for course with this course_code
 * See example output from assignment handout for formatting.
 *
 */
int stats_by_course(Student *stu_list, char *course_code, Course *courses, int num_courses, Ta *ta_list) {

    // TODO: students will complete these next pieces but not all of this
    //       function since we want to provide the formatting
    Course *found = find_course(courses, num_courses, course_code);
    if (found == NULL) {
        return 1;
    }
    int students_waiting = 0;
    int students_being_helped = 0;

    Student *student = stu_list;
    while (student != NULL) {
        if (strcmp(student->course->code, course_code)==0) {
            students_waiting++;
        }
        student = student->next_overall;
    }

    Ta *ta = ta_list;
    while(ta != NULL) {
        if (ta->current_student == NULL) {
            ta = ta->next;
        } else if (strcmp(ta->current_student->course->code, course_code)==0) {
            students_being_helped++;
            ta = ta->next;
        } else {
            ta = ta->next;
        }
    }

    // You MUST not change the following statements or your code
    //  will fail the testing.

    printf("%s:%s \n", found->code, found->description);
    printf("\t%d: waiting\n", students_waiting);
    printf("\t%d: being helped currently\n", students_being_helped);
    printf("\t%d: already helped\n", found->helped);
    printf("\t%d: gave_up\n", found->bailed);
    printf("\t%f: total time waiting\n", found->wait_time);
    printf("\t%f: total time helping\n", found->help_time);

    return 0;
}


/* Dynamically allocate space for the array course list and populate it
 * according to information in the configuration file config_filename
 * Return the number of courses in the array.
 * If the configuration file can not be opened, call perror() and exit.
 */
int config_course_list(Course **courselist_ptr, char *config_filename) {
    FILE *config = fopen(config_filename, "r");
    if (config == NULL) {
        perror("Could not open file");
        exit(1);
    }

    int num_courses = 0;
    char line[INPUT_BUFFER_SIZE];
    fgets(line, sizeof(line), config);
    num_courses = atoi(line);
    *courselist_ptr = malloc(num_courses * sizeof(Course));

    for (int i=0;i<num_courses;i++) {
        fgets(line, INPUT_BUFFER_SIZE, config);
        sscanf(line, "%6s", (*courselist_ptr)[i].code);

        char description[INPUT_BUFFER_SIZE];
        strncpy(description, line+7, INPUT_BUFFER_SIZE);
        description[INPUT_BUFFER_SIZE-1] = '\0';
        (*courselist_ptr)[i].description = malloc(strlen(description)+1);
        strcpy((*courselist_ptr)[i].description, description);
    }

    fclose(config);
    return num_courses;
}
