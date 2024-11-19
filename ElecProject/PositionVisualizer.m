data = readtable("output.csv");
scatter3(data, "position_x", "position_z", "position_y", "filled", "ColorVariable", "ElapsedTime");
set(gca, 'Projection', 'Perspective');

cb = colorbar;
cb.Label.set("String", 'Simulation Time (s)');
xlabel('X-Pos');
ylabel('Z-Pos');
zlabel('Y-Pos');
