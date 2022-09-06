data = csvread('./acc_gyro_data.csv');

% CALCULATE ANGLE FROM GYROSCOPE DATA
gyro = zeros(1, size(data,1));
gyro(1) = 0;
gyro_sum = 0;
for i = 2:size(data,1)
    gyro_sum = gyro_sum + (data(i,2) + data(i-1,2))/2;
    gyro(i) = gyro_sum/1130;
end

% KALMAN FILTER IMPLEMENTATION
h = 0.05;

x = [0, 0]';
P = [10, 0;
     0, 10];

F = [1, -h;
     0, 1];
G = [h, (-h*h)/2;
     0, h];
H = eye(2);

b = 0.01;
R = b*eye(2);
Q = eye(2);
 
for i = 1:size(data,1)
    K = (F*P(:,:,i)*H')/(R + H*P(:,:,i)*H');
    %{
    p11 = P(1,1,i);
    p12 = P(1,2,i);
    p21 = P(2,1,i);
    p22 = P(2,2,i);
 
    detK = 1/(b*b + b*p11 + b*p22 + p11*p22 - p12*p21);
    K = [ ((p11 - h*p22)*(b + p22) - p21*(p12 - h*p22))*detK,  (-p12*(p11 - h*p21) + (b + p11)*(p12 - h*p22))*detK  ;
          (p21*(b + p22) - p21*p22)*detK,                      (-p12*p21 + p22*(b + p11))*detK                      ];
    %}
  
    x(:,i+1)  = F*x(:,i) + K*([data(i,1),gyro(i)]'-H*x(:,i));
    %{
    k11 = K(1,1);
    k12 = K(1,2);
    k21 = K(2,1);
    k22 = K(2,2);
    
    x1 = x(1,i);
    x2 = x(2,i);
    y1 = data(i,1);
    y2 = gyro(i);
    x(:,i+1) = [ x1 - h*x2 + k11*(y1-x1) + k12*(y2-x2) ;
                 x2 + k21*(y1-x1) + k22*(y2-x2)        ];
    %}
    
    P(:,:,i+1) = F*P(:,:,i)*F' + G*Q*G' - K*H*P(:,:,i)*F';
    %{
    aa = p11 - h*p21 - h*(p12-h*p22) + h*h + (h*h*h*h)/4 - (k11*p11 + k12*p21 - h*(k11*p12 + k12*p22));
    bb = p12 - h*p22 - (h*h*h)/2 - (k11*p12 + k12*p22);
    cc = p21 - h*p22 - (h*h*h)/2 - (k21*p11 + k22*p21 - h*(k21*p12 + k22*p22));
    dd = p22 + h*h   - (k21*p12 + k22*p22);
    
    P(:,:,i+1) = [aa, bb; cc, dd];    
    %}
end

% SHOW DATA
n = size(data,1);
figure
plot(1:n, data(:,1), 'b')
hold on
plot(1:n, gyro, 'g')
hold on
plot(1:n+1, x(1, :), 'r')
legend('accel','gyro', 'kalman')