# YA65C02 - yet another 65C02 emulator

YA65C02 is an emulator of the 65C02 processor. For the sake of simplicity its behavior is not emulated accurately (e.g. the timing calculation does not account for crossing page boundary). The code seems to run correctly, it passes the [functional tests of Klaus Dormann](https://github.com/Klaus2m5/6502_65C02_functional_tests) and runs the binary of the [Mephisto MMV](https://www.schach-computer.info/wiki/index.php?title=Mephisto_MM_V) chess computer (binary available [here](http://rebel13.nl/rebel13/rebel%2013.html "Ed Schröder")).

![mmv_emu](https://github.com/d3rvita/ya65c02/blob/main/mmv_emu.png)
