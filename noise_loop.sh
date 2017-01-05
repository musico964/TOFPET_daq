#!/bin/bash
ch_start=27
ch_end=34
template_config=/home/musico/TOPEM/EndoProbe/acq/out/test_150427/noise_scan/config_dark.template

for ((j=$ch_start;j<=$ch_end;j++)); do
	rm -f config_noise.txt
	sed s/__FNAME__/dark_2_ch$j/ $template_config >> config1.txt
	chmask=1
	if [ $j -lt 32 ]; then
		let chmask="$chmask << $j"
		sed s/__CH_MASK0__/$chmask/ config1.txt >> config2.txt
		sed s/__CH_MASK1__/0/ config2.txt >> config_noise.txt
	fi
	if [ $j -gt 31 ]; then
		let chmask="$chmask << ($j-32)"
		sed s/__CH_MASK0__/0/ config1.txt >> config2.txt
		sed s/__CH_MASK1__/$chmask/ config2.txt >> config_noise.txt
	fi
	rm -f config1.txt config2.txt
	./Daq -cnf config_noise.txt
done

