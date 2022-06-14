% Constants (D)
U = 150;
R1 = 200;
R2 = 200;
R3 = 660;
R4 = 200;
R5 = 550;
R6 = 400;

R45 = R4+R5;

R145 = (R1*R45)/(R1+R45)
R62 = (R6*R2)/(R6+R2)
Ri = R145 + R62

Ur1 = U * (R1/(R1+R45))
Ur2 = U * (R2/(R2+R6))
Ue = Ur2 - Ur1

IR3 = Ue/(Ri+R3)
Ur3 = IR3 * R3
