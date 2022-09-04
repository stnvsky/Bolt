M = 0.06;   % [kg]
m = 0.236;  % [kg]
L = 0.07;   % [m]
b = 0.1;    % [Ns/m]
g = 9.81;   % [m/s^2]

Kp1 = -5;
Ki1 = -10.69;
Kd1 = -0.2;

Kp2 = 0.5;
Kd2 = 0.1;

A = [0,  0,  1,  0;
     0,  0,  0,  1;
     0,  (-m*g)/M,         -b/M,     0;
     0,  ((M+m)*g)/(M*L),  b/(M*L),        0];
 
B = [0, 0, 1/M, (-1)/(M*L)]';

C = [0, 1, 0, 0];

D = 0;


%s = tf('s');
%sys1 = ss(A, B, C1, D);
%P1 = tf(sys1);
%pidTuner(P1, 'pid')

%sys2 = ss(A, B, C2, D);
%P2 = 1/((M+m)*(s^2) + b*s);

%PIDtf = Kp1 * (1 + Td1*s + 1/(Ti1*s));
% closedLoop = feedback(PIDtf*P, 1);

%PDtf = Kp2 * (1 + Td2*s);
% rlocus(PDtf*P2)

%figure;
%step(closedLoop)

%rlocus(PIDtf*P)

