cmake_minimum_required(VERSION 3.12)
option(ENABLE_TCP4TLM_TESTS "Include tcp4tlm test" OFF)

find_package(fmt)
find_package(Boost COMPONENTS program_options REQUIRED)
add_executable (tcp4tlm_bridge 
    sc_main.cpp
    initiator.cpp
)
target_link_libraries (tcp4tlm_bridge LINK_PUBLIC scc fmt::fmt)
if(TARGET Boost::program_options)
	target_link_libraries(tcp4tlm_bridge PUBLIC Boost::program_options)
else()
	target_link_libraries(tcp4tlm_bridge PUBLIC ${BOOST_program_options_LIBRARY})
endif()


file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/run_tcp4tlm_bridge_ctest.sh CONTENT [=[
#!/usr/bin/env bash
set -euo pipefail

exe="$1"
port0=$((30000 + ($$ % 10000) * 2))
port1=$((port0 + 1))
log0="${TMPDIR:-/tmp}/tcp4tlm_bridge_${port0}_0.log"
log1="${TMPDIR:-/tmp}/tcp4tlm_bridge_${port1}_1.log"
pids=()

cleanup() {
    for pid in "${pids[@]}"; do
        kill "$pid" 2>/dev/null || true
    done
    for pid in "${pids[@]}"; do
        wait "$pid" 2>/dev/null || true
    done
}

trap cleanup EXIT

"${exe}" --local-port "${port0}" >"${log0}" 2>&1 &
pids+=($!)
"${exe}" --local-port "${port1}" --remote-host localhost --remote-port "${port0}" >"${log1}" 2>&1 &
pids+=($!)

status=0
for pid in "${pids[@]}"; do
    if ! wait "$pid"; then
        status=1
    fi
done

if [[ ${status} -ne 0 ]]; then
    cat "${log0}" "${log1}"
    exit "${status}"
fi

grep -q "FINISHED" "${log0}"
grep -q "FINISHED" "${log1}"
]=])

if(ENABLE_TCP4TLM_TESTS)
    add_test(NAME tcp4tlm_bridge_peer_ctest COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/run_tcp4tlm_bridge_ctest.sh $<TARGET_FILE:tcp4tlm_bridge>)
    set_tests_properties(tcp4tlm_bridge_peer_ctest PROPERTIES TIMEOUT 60)
endif()