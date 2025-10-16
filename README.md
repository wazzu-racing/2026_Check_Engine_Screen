# Check_Engine_Screen

Units of values:
afr 
fuelload 
o.spark_advance
o.baro = row[20]/ 1000;
o.map = row[21]/ 1000;
o.mat = row[22]/ 1000;
o.clnt_temp = row[23]/ 1000;
o.tps = row[24]/ 1000;
o.batt = row[25]/ 1000;
o.oil_press = row[26]/ 1000;
o.ltcl_timing = row[27]/ 1000;
o.ve1 = row[28]/ 1000;
o.ve2 = row[29]/ 1000;
o.egt = row[30]/ 1000;
o.maf = row[31]/ 1000;
o.in_temp = row[32]/ 1000;
o.ax = row[33]/ 1000;
o.ay = row[34]/ 1000;
o.az = row[35]/ 1000;
o.imu_x = row[36]/ 1000;
o.imu_y = row[37]/ 1000;
o.imu_z = row[38]/ 1000;
o.susp_pot_1 = ((((row[39]/5024))/(1))*(100))/25.4;
o.susp_pot_2 = ((((row[40]/5024))/(1))*(100))/25.4;
o.susp_pot_3 = ((((row[41]/5024))/(1))*(100))/25.4;
o.susp_pot_4 = ((((row[42]/5024))/(1))*(100))/25.4;
o.rad_in = (((row[43]/5024)-0.5232)/(0.0084-0.5232))*(302+58)-58;
o.rad_out = (((row[44]/5024)-0.5232)/(0.0084-0.5232))*(302+58)-58;
o.amb_air_temp = row[45];
o.brake1 = 5000*(((row[46]/5024.0)-0.1)/0.8);
o.brake2 = 5000*(((row[47]/5024.0)-0.1)/0.8);
