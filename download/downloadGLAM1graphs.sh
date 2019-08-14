YYYY=2019
#CRP/NUTS2
#PL,CZ,AU,SK
#for crp in 37257 37256 37255 37254 37071 37075 37212 37215 37211 37213 37214 37216 37217 37221 37220 37224 37223 37222 37225 37219 37232 37231 37227 37233 37226 37218 37230 37229 37228 37210 37072
#BU,HU,RO
#for crp in 37082 37084 37083 37081 37080 37079 38337 38338 38346 38351 38355 38323 38327 38318 38027 38031 38022 38015 38014 38024 38019
#Greece
#for crp in 37161 37157 37159 37160 37156 37155 37154 37153 37162 37165 37164
#IT
#for crp in 37175 37178 37181 37180 37179 37182 37183 37177 37184 37185 37186 37187 37188 37190 37189 37191 37192 37193 37194 37195
#DE
#for crp in 37116 37117 37124 37118 37119 37120 37121 37122 37114 37115 37145 37146 37144 37139 37138 37140 37137 37136 37135 37141 37142 37143 37131 37130 37123 37132 37134 37152 37150 37127 37126 37133 37151 37149 37147 37148
#FR
#for crp in 37104 37103 37097 37095 37094 37099 37100 37093 37098 37102 37101 37092 37105 37096 37108 37106 37110 37107 37111 37109 37112
#SP+PT
count=0
countpdf=0
for crp in 37273 37267 37268 37266 37259 37260 37261 37262 37263 37264 37269 37270 37272 37235 37238 37236 37234 37258 37265
do
	wget -nv -c https://glam1.gsfc.nasa.gov/_/plot1_2.2?src=https%3A%2F%2Fglam1.gsfc.nasa.gov%2Ftbl%2Fv4%2FMYD%2FGMYD09Q1.6v1.NDVI.MOD44W_2009_land.ADM%2Ftxt%2FGMYD09Q1.6v1.NDVI.MOD44W_2009_land.ADM.$crp.tbl.txt\&plotmu=1\&minmax=3\&yrs=$YYYY\&smonth=1\&nmonth=12\&type=ADM\&save=1 --output-document=$YYYY.$crp.png
done
convert $(ls *png) SPPT.$YYYY.pdf
