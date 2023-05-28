Scripts serve to build / clean all examples at once.  

`build_all_clangd.sh` uses bear to additionally generate a `compile_commands.yaml` for each example so the C/C++ language server clangd can be aware of the paths in the makefiles.
