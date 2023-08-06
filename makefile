MAKEFLAGS += --no-print-directory

bboost:
	cd boost && ./bootstrap.sh --with-libraries=program_options,system,thread --with-toolset=clang && ./b2 toolset=clang-12 cxxflags="-std=c++17" link=static
dgenerate:
	cmake -DCMAKE_CXX_FLAGS="-fno-limit-debug-info" -DCMAKE_CXX_FLAGS_DEBUG="-ggdb3" -DCMAKE_C_COMPILER=clang-12 -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=Debug -S . -B build/Debug

rgenerate:
	cmake -DCMAKE_C_COMPILER=clang-12 -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=Release -S . -B build/Release

rdgenerate:
	cmake -DCMAKE_C_COMPILER=clang-12 -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=RelWithDebInfo -S . -B build/RelWithDebInfo

dbuild:
	cmake --build build/Debug -- -j 12

rbuild:
	cmake --build build/Release -- -j 12

rdbuild:
	cmake --build build/RelWithDebInfo -- -j 12
