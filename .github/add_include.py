import os.path
Import("env", "projenv")
# Rely on the fact that all examples to build_src_filter = +<EXAMPLE_THEY_WANT>
# And figure out correct include path from that
src_filter = str(env.subst("$SRC_FILTER"))
# A string with e.g. "+<ch32v003fun> +<examples/blink>" 
# rudimentary parsing is fine for CI
src_filters = src_filter.split("+")
example_folder = ""
for filter in src_filters:
   # cleanup unwanted characters
   f = filter.replace("<", "", 1).replace(">", "", 1)
   # starts with "examples" and looks like a path?
   if f.startswith("examples") and ("/" in f or os.path.sep in f):
      example_folder = f
      break
   
# propagate to all construction environments
for e in env, projenv, DefaultEnvironment():
   e.Append(CCFLAGS=[("-I", example_folder)])
