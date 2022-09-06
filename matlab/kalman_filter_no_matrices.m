clear
%data = csvread('./acc_gyro_data.csv');
data = csvread('./data2.csv');

% CALCULATE ANGLE FROM GYROSCOPE DATA
gyro = zeros(1, size(data,1));
gyro(1) = 0;
gyro_sum = 0;
for i = 2:size(data,1)
    gyro_sum = gyro_sum + (data(i,2) + data(i-1,2))/2;
    gyro(i) = gyro_sum/1130;
end

angle_lowpass = zeros(1, size(data,1));
angle_lowpass(1) = 0;
for i = 2:size(data,1)
    angle_lowpass(i) = 0.828 * angle_lowpass(i-1) + 0.0861* data(i,1) + 0.0861 * data(i-1,1);
end



% KALMAN FILTER IMPLEMENTATION
h = 0.05;
b = 0.01;

x1 = 0;
x2 = 0;

p11 = 10;
p12 = 0;
p21 = 0;
p22 = 10;

x1_show = zeros(1, size(data,1));
x2_show = zeros(1, size(data,1));

for i = 1:size(data,1)
    x1_show(i) = x1;
    x2_show(i) = x2;
    detK = 1/(b*b + b*p11 + b*p22 + p11*p22 - p12*p21);
    k11 = ((p11 - h*p22)*(b + p22) - p21*(p12 - h*p22))*detK; 
    k12 = (-p12*(p11 - h*p21) + (b + p11)*(p12 - h*p22))*detK;
    k21 = (p21*(b + p22) - p21*p22)*detK;                     
    k22 = (-p12*p21 + p22*(b + p11))*detK;

    y1 = angle_lowpass(i);%data(i,1);
    y2 = data(i,2);%gyro(i);
    x1 = x1 - h*x2 + k11*(y1-x1) + k12*(y2-x2);
    x2 = x2 + k21*(y1-x1) + k22*(y2-x2);
    
    next_p11 = p11 - h*p21 - h*(p12-h*p22) + h*h + (h*h*h*h)/4 - (k11*p11 + k12*p21 - h*(k11*p12 + k12*p22));
    next_p12 = p12 - h*p22 - (h*h*h)/2 - (k11*p12 + k12*p22);
    next_p21 = p21 - h*p22 - (h*h*h)/2 - (k21*p11 + k22*p21 - h*(k21*p12 + k22*p22));
    next_p22 = p22 + h*h   - (k21*p12 + k22*p22);
    
    p11 = next_p11;
    p12 = next_p12;
    p21 = next_p21;
    p22 = next_p22;    
end

% SHOW DATA
n = size(data,1);
figure
plot(1:n, data(:,1)*57.3248, 'b')
hold on
plot(1:n, data(:,2)*57.3248, 'g')
hold on
%{
plot(1:n, x1_show*57.3248, 'r')
legend('accel','gyro', 'x1')
grid on


% SHOW LOW PASS FILTER RESULT
figure 
plot(1:n, angle_lowpass*57.3248, 'r')
hold on
plot(1:n, data(:,1)*57.3248, 'b')
grid on
%}