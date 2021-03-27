==============================================
Creating codec to codec dai link for ALSA dapm
==============================================

Mostly the flow of audio is always from CPU to codec so your system
will look as below:
::

   ---------          ---------
  |         |  dai   |         |
      CPU    ------->    codec
  |         |        |         |
   ---------          ---------

In case your system looks as below:
::

                       ---------
                      |         |
                        codec-2
                      |         |
                      ---------
           