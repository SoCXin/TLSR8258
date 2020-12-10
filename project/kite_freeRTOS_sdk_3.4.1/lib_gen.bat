
echo 编译 ble_remote 工程后，手工运行 lib_gen.bat会将适当的 .o 文件打包成 .a

rm -f ./proj_lib/liblt_8258.a 
find ./8258_ble_remote/drivers -name 'pm.o' -type f -print -exec tc32-elf-ar -r ./proj_lib/liblt_8258.a {} ;
find ./8258_ble_remote/drivers -name 'pm_32k_rc.o' -type f -print -exec tc32-elf-ar -r ./proj_lib/liblt_8258.a {} ;
find ./8258_ble_remote/drivers -name 'pm_32k_xtal.o' -type f -print -exec tc32-elf-ar -r ./proj_lib/liblt_8258.a  {} ;
find ./8258_ble_remote/drivers -name 'rf_drv.o' -type f -print -exec tc32-elf-ar -r ./proj_lib/liblt_8258.a {} ;
find ./8258_ble_remote/drivers -name 'random.o' -type f -print -exec tc32-elf-ar -r ./proj_lib/liblt_8258.a {} ;
find ./8258_ble_remote/stack -name '*.o' -type f -print -exec tc32-elf-ar -r ./proj_lib/liblt_8258.a {} ; 

