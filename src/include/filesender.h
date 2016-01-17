//REV: 17 Jan 2016. Header/functions to handle
//sending of file via MPI. We can separate MPI vs Hadoop etc.
//


//Goal is to go from PARAMPOINT to PARAMPOINT on other side. Transformation is
//LOCATION (e.g. locally it has /TMP as its guy, we want to pass EVERYTHING?)
//Or only output? We don't care about e.g. local outputs? Copy everything?


//It's only a single PITEM though! So we only need to copy that PITEM stuff, i.e. the base dir of the PITEM that I just computed.
//I iterate through the directory and copy over all the files, or only the output files? Much easier to only copy the output files.
//But, in that case I will lose STDIN/STDOUT etc. However, I can artificially add "STDIN" and "STDOUT" to "success" files even if nothing will
//be printed to them? Whatever, fine. What will the guy output? It is part of the blah? I don't know it might output some other files I don't know
//about or don't care about? Like network structure etc. Whatever force user to specify it all for now, for ease.

//Only grab those files. We need to know what their corresponding location at target is. We will copy them all to "this guys" directory on the
//master host .

//So, user specifies "required" files and "success" files. Success files will ALWAYS be relative to source? Or can it force it to produce elsewhere?
//Assume they are always relative to source of "this" active pitem? Need way of getting "necessary" guys from previous psets. That's fine but whatever.
//We need to modify those guys too (remember!). I.e. their location is not local, but based in /TMP instead of elsewhere in filesystem...
//Anyway, somehow we have those values set? Not generated, but set?

//What are actual expected use scenarios?

//In MASTER
//1) I have "required" files in "data" folder somewhere on the system
//2) I have "required" files in of a PREVIOUS pset in multiple folders
//3) I have "required" files in the CURRENT directory (?)

//These need to all be copied appropriately, and references to them changed to cope with SLAVEs expected locations.

//Literally they are just "success" files. The best way is to probably give them individual names so we know their correspondences. Easiest.
//However, I do not expect to be overwriting any files. Thus, it is easiest to just copy back. Of course DATA guys are um, annoying. Need to change
//only their directories, not filenames? How to deal with DATA etc? Yea, give it a way to copy reused data files at beginning. And refer to them
//that way. I.e. data files won't be recopied. At any rate, we go through REQUIRED files, taking their current location (always referenced from
//parampoint base?!?!?! or else we have problems. Assume user doesn't overwrite them?) Need to re-build the whole parampoint guy ON THE OTHER SIDE,
//using a different base. That makes it much easier. Assume I didn't move the files. So, then the only thing I need to do is copy to corresponding
//location? But in old pset, we don7t know their old or new locations? So, only option is correspondingly copy it? If we know it will be old pset guy,
//copy it over somehow? We need to give it "new" location, always offset from base? Just take relative to base if it is in our dir, and put it in
//corresponding dir over here? Pain in the butt to re-build a dir hierarchy each time..?? Waste of resources... better to change filenames?
//And put them all in one place? I guess... Any reason to have full guy? have "data" target, and "required" files all in one place? But if they
//are all named "output"? Easy, just check of name collision. We need to know where that happens in the current "CMD" though. So we change the
//corresponding filename in the generated guy? All required files will be where? Easier to rerun build of paramset. Even if we send it, we need
//to tell it what name it was? Might have filenames hardcoded into variables or something in the PARAMPOINT generator, or as an array, in which
//case we're fucked? No, they can't be, they must be single ones in REQUIRED/SUCCESS, so I can move them over. However, we might refer to them differently?
//In which case we have to change all their other references, unless we always refer to them "through" success file array. Which is not what happens.
//So, yappa, I have to rebuild it...with a different base dir. And when I copy it over, I need to tell it relative thing to base for non-data files,
//and just leave data files as are I guess. Can't require anything outside of that I guess... OK go.

//REV: JUST IMPL IT

//In SLAVE
//After completion I have "success" files (output files?) in the TMP dir. Mostly (probably) in the current PITEM guy.

//These need to be copied appropriately back to MASTER
