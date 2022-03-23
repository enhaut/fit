% Zadání è. 24:
% Napište program øešící úkol daný predikátem u24(LIN,VIN1,VIN2,VOUT), kde 
% LIN je vstupní èíselný seznam, promìnné VIN1 a VIN2 obsahují èísla 
% splòující podmínku VIN1>VIN2 a VOUT je promìnná, ve které se vrací první 
% èíslo seznamu LIN splòující podmínku VIN1>VOUT>VIN2. Pokud žádné takové 
% èíslo neexistuje je predikát nepravdivý (vrací hodnotu false).  

% Testovací predikáty:                         			% VOUT        
u24_1:- u24([15,2,4,9,12,17],10,2,VOUT),write(VOUT).		% 4
u24_2:- u24([15,2,-14,9,12,17],10,2,VOUT),write(VOUT).		% 9
u24_3:- u24([-10,-20.8,-5.3,0,7],0,-10,VOUT),write(VOUT).	% -5.3
u24_r:- write('Zadej LIN: '),read(LIN),
        write('Zadej VIN1: '),read(VIN1),
        write('Zadej VIN2: '),read(VIN2),
        u24(LIN,VIN1,VIN2,LOUT),write(LOUT).


u24(LIN,VIN1,VIN2,VOUT):-
