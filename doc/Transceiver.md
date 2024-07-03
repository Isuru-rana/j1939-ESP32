# CAN Transceivers

What?  I need extra hardware?  Why!

Well, CAN bus has some pretty nifty tricks up its sleeve:

* Can run dozens, hundreds of meters with minimal errors
* Quick-ish at up to 1Mbit[^1]
* No specific master/slave requirement

To pull all this off, we need something a little more robust than regular logic
level signals.  I'm a pretty amateur EE so I'll save how all that works for the
big boys, but suffice to say you need the extra oomph a transceiver provides.

1.  Newer can standards like CAN-FD and CAN-XL go way faster, but those are not covered here