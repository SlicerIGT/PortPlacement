set(DAVINCI_PARAMETERS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/davinci-parameters.xml") 

add_executable(da-vinci-params-load-test da-vinci-params-load-test.cxx)
target_link_libraries(da-vinci-params-load-test
  davinci
  ${NLopt_LIBRARIES}
  ${VTK_LIBRARIES}
  )
add_test(da_vinci_load_params_test da-vinci-params-load-test ${DAVINCI_PARAMETERS_FILE})

add_executable(intra-ik-test intra-ik-test.cxx)
target_link_libraries(intra-ik-test
  davinci
  ${NLopt_LIBRARIES}
  ${VTK_LIBRARIES}
  )
add_test(da_vinci_intra_ik_test intra-ik-test ${DAVINCI_PARAMETERS_FILE})

add_executable(jacobian-test jacobian-test.cxx)
target_link_libraries(jacobian-test
  davinci
  ${NLopt_LIBRARIES}
  ${VTK_LIBRARIES}
  )
add_test(da_vinci_jacobian_test jacobian-test ${DAVINCI_PARAMETERS_FILE})

add_executable(da-vinci-passive-ik-test da-vinci-passive-ik-test.cxx)
target_link_libraries(da-vinci-passive-ik-test
  davinci
  ${NLopt_LIBRARIES}
  ${VTK_LIBRARIES}
  )
add_test(da_vinci_passive_ik_test da-vinci-passive-ik-test ${DAVINCI_PARAMETERS_FILE})

add_executable(min-distance-test min-distance-test.cxx)
target_link_libraries(min-distance-test collisions)
add_test(collisions_distance_test min-distance-test ${DAVINCI_PARAMETERS_FILE})

# add_executable(config-ut-test config-ut-test.cxx)
# target_link_libraries(config-ut-test davinci)
# add_test(da_vinci_config_ut_test config-ut-test ${DAVINCI_PARAMETERS_FILE})

add_executable(find-feasible-plan-test find-feasible-plan-test.cxx)
target_link_libraries(find-feasible-plan-test
  ${NLopt_LIBRARIES}
  optim
  ${VTK_LIBRARIES}
  )
add_test(optim_find_feasible_plan_test find-feasible-plan-test ${DAVINCI_PARAMETERS_FILE})
