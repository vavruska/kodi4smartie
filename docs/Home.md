# KODI4SMARTIE
::![](Home_kodi4smartie0.png)

KODI4SMARTIE is a DLL for LCD Smartie ([http://lcdsmartie.sourceforge.net/](http://lcdsmartie.sourceforge.net/)) that allows information about what is playing in Kodi ([http://kodi.tv](http://kodi.tv)) to be displayed on an LCD Character display. See the LCD Smartie web site for supported devices.

+My Setup+
Windows 7 x64
LCD Smartie 5.4
Kodi v16.1 (Jarvis) 
2x20 Blue back-lit character display using a SPLC780C (HD44780 compatible) controller.
PIC LCD Backpack ([http://dangerousprototypes.com/docs/PIC_LCD_backpack](http://dangerousprototypes.com/docs/PIC_LCD_backpack))

I connect directly to one of the USB headers on my motherboard. 

**NOTE:** If you are not connecting the PIC LCD Backpack directly to your motherboard then make sure you have the proper cable. The PIC LCD Backpack uses a MINI-B USB cable, not a MICRO-B USB cable. The MICRO-B USB cables are the standard these days and will not fit the PIC LCD Backpack. It is a close fit but will not work.

Kodi4Smartie uses the C++ REST SDK ([https://casablanca.codeplex.com/](https://casablanca.codeplex.com/)).


