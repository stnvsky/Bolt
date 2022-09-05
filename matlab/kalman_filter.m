% SHOW DATA
T = readtable('./acc_gyro_data.csv');
data = table2array(T);

gyro = zeros(1, size(data,1));

gyro(1) = 0;
gyro_sum = 0;
for i = 2:size(data,1)
    gyro_sum = gyro_sum + (data(i,2) + data(i-1,2))/2;
    gyro(i) = gyro_sum/1130;
end

%figure
%plot(1:size(data,1), data(:,1), 'r')
%hold on
%plot(1:size(data,1), gyro, 'b')

% KALMAN FILTER IMPLEMENTATION
dt = 1;
x = [0, 0]';  % [omega, theta]
P = [0, 0;
     0, 0];
 
A = [1, -dt;
     0, 1];
H = [1, 0;
     0, 0];
Q = [0, 0;
     0, 0];
R = [0, 0;
     0, 0];
 
% implementation
for i = 2:size(data,1)
    x(:,i) = A*x(:,i-1);
    P(:,:,i) = A*P(:,:,i-1)*A' + Q;
    K = (P(:,:,i)*H')\(H*P(:,:,i)*H' + R);
    x(:,i) = x(:,i) + K*H'*(data(i,:)' - H*x(:,i));
    P(:,:,i) = P(:,:,i) - K*H*P(:,:,i);
end

x(:,500)