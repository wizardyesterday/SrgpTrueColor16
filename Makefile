default :
	@echo MAKING... SRGP optimized library 
	(cd src/srgp;  make clean CFLAGS=-O; make)
	@echo MAKING... SPHIGS optimized library 
	(cd src/sphigs;  make clean CFLAGS=-O; make)
	@echo MAKING... example programs
	(cd examples; make)
	(cd examples; make PROG=robot_anim)
#	(cd doc; make)
