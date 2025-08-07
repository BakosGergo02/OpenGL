#version 430

uniform bool isLine;
uniform vec3 center;
out vec4 color;
double dist;
void main()
{
	if(isLine)
	{
		color = vec4(0,0,1,0);
	}
	else
	{
		dist = sqrt(pow(center.x+300-gl_FragCoord.x,2) + pow(center.y+300-gl_FragCoord.y,2));
		color = vec4(1-dist/50, dist/50, 0.0, 1.0);
	}
}
