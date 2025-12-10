add_executable(wordcount-gbench
    ${WC}/params.hxx
    ${WC}/utils.cxx

    ${WC}/baseline.cxx
    ${WC}/using-reserve.cxx
    ${WC}/char-fn.cxx
    ${WC}/mem-map-file.cxx

    wordcount-gbench.cxx
)
target_compile_options(wordcount-gbench PRIVATE -O3 -march=native)
target_include_directories(wordcount-gbench PRIVATE ${WC})
target_link_libraries(wordcount-gbench PRIVATE
    benchmark::benchmark
)


add_executable(charfn-gbench
    char-fn-gbench.cxx
)
target_compile_options(charfn-gbench PRIVATE -O3 -march=native)
target_include_directories(charfn-gbench PRIVATE ${WC})
target_link_libraries(charfn-gbench PRIVATE
    benchmark::benchmark
)

