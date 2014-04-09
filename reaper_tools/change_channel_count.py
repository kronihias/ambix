# this is a Python ReaScript for use with the DAW Reaper
# it changes the number of channels for all selected tracks 
# including their sends to a number given by the user
#
# this is very handy if you want to change ambisonic orders on the fly 
# to save cpu processing (mainly channel copying/adding stuff)
# and changing to full resolution just before rendering
#
#
# example use: you have several channels with ambisonic encoders
# sending to a main ambisonic bus
#
# select the ambi bus AND all encoder channels -> execute this script from Actions menu
#
# INSTALL:
# copy this to the ReaScript Folder, OSX: ~/Library/Application Support/REAPER/Scripts/
# REFERENCE: http://forum.cockos.com/showthread.php?t=131797

# by Matthias Kronlachner and Daryl Pierce

def msg(m):
	RPR_ShowConsoleMsg(str(m) + "\n")

def is_odd(num):
    return num & 0x1

selectedTrackCount = RPR_CountSelectedTracks(0)

if selectedTrackCount > 0:
	chan = RPR_GetUserInputs("Channels",1,"how many track channels?","",64)[4]
	
	chan = int(chan.strip())
	
    # is chan even? if not make it even!
	if is_odd(chan):
		chan = chan + 1
		msg("you entered an oddchannel number, this is not allowed. corrected channel number to:")
		msg(chan)
        
	for i in range(selectedTrackCount):
		selectedTrack = RPR_GetSelectedTrack(0, i)
		maxlen = 1024*1024*4
		# Get track state chunk
		chunk = RPR_GetSetTrackState(selectedTrack, "", maxlen)[2]
		# msg(chunk)
		
		# how many channels per track?
		num_channels = chan
		
		# msg(num_channels)
		
		# format a new NCHAN line
		nchan = "NCHAN %s\n" % num_channels
		
		# simple regular expression that
		# substitutes in the new NCHAN line ...
		chunk = re.sub(r"NCHAN\s\d+\n", nchan, chunk)
		
		ReceiveList = re.findall("AUXRECV.*",chunk)
		subcount = len(ReceiveList)
		recvCount = subcount - 1

    # iterate through auxrecvs
		while 0 <= recvCount: 
			recvValues = ReceiveList[recvCount]
			valuelist = re.split("\s",recvValues)
			recvVal = 512 * int(chan)
			valuelist[8] = str(recvVal) # audio: src channel count * 1024

			valuestring = ' '.join(valuelist)
			chunk = re.sub("AUXRECV.*", valuestring, chunk, subcount)

			recvCount -= 1
			subcount -= 1
			
		RPR_GetSetTrackState(selectedTrack, chunk, maxlen)