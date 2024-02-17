from os.path import join
Import("env")

mcu_package = 1
target_mcu = 0
mcu_ld = 0
linkerscript_cmd = env.Command(
    join("$BUILD_DIR", "ldscript.ld"),  # $TARGET
    join(env.subst("$PROJECT_DIR"), "ch32v003fun", "ch32v003fun.ld"),  # $SOURCE
    env.VerboseAction(" ".join([
        "$CC",
        "-E",
        "-P",
        "-x",
        "c",
        "-DTARGET_MCU=%d" % target_mcu,
        "-DMCU_PACKAGE=%d" % mcu_package,
        "-DTARGET_MCU_LD=%d" % mcu_ld,
        "$SOURCE" 
        ">",
        "$TARGET"
    ]), "Generating linkerscript $BUILD_DIR/ldscript.ld")
)
env.Depends("$BUILD_DIR/${PROGNAME}.elf", linkerscript_cmd)
env.Replace(LDSCRIPT_PATH=join("$BUILD_DIR", "ldscript.ld"))