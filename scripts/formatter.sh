# format python code
uvx ruff format

# format C/C++ code
find . \
  \( -path './000_thirdparty' -o -path './.venv' \) -prune -false \
  -o \
  -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0 \
| xargs -0 clang-format -i
