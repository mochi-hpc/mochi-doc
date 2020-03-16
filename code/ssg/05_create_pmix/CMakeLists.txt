if (PMIX_FOUND)
	#  add_executable (05_ssg_create_pmix main.c)
	#  target_link_libraries (05_ssg_create_pmix ssg margo pmix)
endif (PMIX_FOUND)
