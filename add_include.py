from os.path import join
Import("env", "projenv")
# propagate to all construction environments
for e in env, projenv, DefaultEnvironment():
   e.Append(CCFLAGS=[("-I", join("examples", env.subst("$PIOENV")))])
