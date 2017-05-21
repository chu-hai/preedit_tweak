find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
find_program(GETTEXT_MSGFMT_EXECUTABLE   msgfmt)

if(NOT GETTEXT_XGETTEXT_EXECUTABLE )
	message(FATAL_ERROR "Command not found: xgettext")
endif()
if(NOT GETTEXT_MSGMERGE_EXECUTABLE )
	message(FATAL_ERROR "Command not found: msgmerge")
endif()
if(NOT GETTEXT_MSGFMT_EXECUTABLE )
	message(FATAL_ERROR "Command not found: msgfmt")
endif()


# Generate POT file
set(POTFILE ${PROJECT_SOURCE_DIR}/po/${PACKAGE}.pot)

add_custom_target(pot_update
	COMMAND	${GETTEXT_XGETTEXT_EXECUTABLE}
		--directory=${PROJECT_SOURCE_DIR}
		--package-name=${PACKAGE}
		--package-version=${PACKAGE_VERSION}
		--language=C
		--force-po
		--from-code=UTF-8
		--sort-by-file
		--keyword=_ --keyword=Q_:1g --keyword=N_ --keyword=C_:1c,2 --keyword=NC_
		--output=${POTFILE}
		${PLUGIN_SOURCES}
 	COMMENT	"Extract translatable messages to ${PACKAGE}.pot"
)


# Update PO file and MO file
file(GLOB POFILES ${PROJECT_SOURCE_DIR}/po/*.po)
set(_pofiles)
set(_gmofiles)
foreach(_pofile ${POFILES})
	get_filename_component(LANG ${_pofile} NAME_WE)
	add_custom_command(
		OUTPUT	${_pofile}
        COMMAND ${CMAKE_COMMAND} -E touch ${_pofile}
		COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
			--backup=none -q
			--update ${_pofile}
			${POTFILE}
		DEPENDS	${POTFILE}
		COMMENT "Update PO file. [${LANG}.po]"
	)

	set(_gmofile ${CMAKE_CURRENT_BINARY_DIR}/${LANG}.gmo)
	add_custom_command(
		OUTPUT	${_gmofile}
		COMMAND	${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmofile} ${_pofile}
		DEPENDS	${_pofile}
		COMMENT "Update MO file. [${LANG}.gmo]"
	)

	install(
		FILES ${_gmofile}
		DESTINATION ${PACKAGE_LOCALE_DIR}/${LANG}/LC_MESSAGES/ RENAME ${GETTEXT_PACKAGE}.mo
	)

	list(APPEND _pofiles  ${_pofile})
	list(APPEND _gmofiles ${_gmofile})
endforeach()

add_custom_target(po_update
	DEPENDS	${_pofiles}
)

add_custom_target(mo_update ALL
	DEPENDS ${_gmofiles}
)
