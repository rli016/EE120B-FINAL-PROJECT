# EE120B-FINAL-PROJECT

HIGH LEVEL DESCRIPTION

Specs for Alarm Clock

	LCD to display time
	
	Button press to tell clock that user is setting time
	
	Button press to tell clock that user is setting alarm 
	
	Input of numbers to represent time
	
	Buzzer goes off for alarm
	
	Button for snooze, snooze capability
	
	Extra: visual cue when alarm goes off for hearing impaired 

Buttons used:

A0: Increases the time at the current position

A1: Moves the cursor to the left by one position. If cursor is at the end, resets to the farthest right position

A2: Goes to SET TIME, using A0 and A1 to set the time, and pressing A2 again to finalize.

A3: Goes to SET ALARM, using A0 and A1 to set the alarm, and pressing A3 again to finalize. Alarm defaults to 12:00:00am if not set

A4: Toggles the alarm. Alarm is set to disable on initialization. 

Speaker is wired to B6.

