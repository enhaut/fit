%%%%% Constants
% Voltages
U1 = 125;
U2 = 65;
% Resistors
R1 = 510;
R2 = 500;
R3 = 550;
R4 = 250;
R5 = 300;
R6 = 800;
R7 = 330;
R8 = 250;

%%%%% Formulas
% 2,
Uekv  = U1 + U2

% 1,
R34   = (R3*R4)/(R3+R4)

% 2,
R234  = R2+R34

% 4,
RA    = (R1*R234)/(R1+R5+R234)
RB    = (R1*R5)/(R1+R5+R234)
RC    = (R5*R234)/(R1+R5+R234)

% 5,
RB7   = RB+R7
RC6   = RC+R6

% 6,
RB7C6 = (RB7*RC6)/(RB7+RC6)

% 7,
Rekv  = RA+RB7C6+R8

% 8,
I     = Uekv/Rekv

% 9,
URA   = RA * I
URB7C6= RB7C6 * I
UR8   = R8 * I
IB7   = URB7C6/RB7
IC6   = URB7C6/RC6

% 10,
I6    = IC6
UR6   = R6 * I6