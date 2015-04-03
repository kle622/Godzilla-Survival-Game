uniform vec3 UaColor;
uniform vec3 UdColor;
uniform vec3 UsColor;
uniform vec3 uLightPos;
uniform float Ushine;
uniform vec3 cam_pos;
varying vec4 normal;
varying vec4 vertex;


void main()
{
	vec3 lightColor = vec3(1, 1, 1);
	vec3 lightVec = normalize(uLightPos - vertex.xyz);
	vec3 halfA = normalize(cam_pos - vertex.xyz);

	vec3 diffuse = lightColor * dot(normal.xyz, lightVec) * UdColor;
	vec3 specular = lightColor * pow(dot(normal.xyz, halfA), Ushine) * UsColor;
	vec3 ambient = UaColor;

	gl_FragColor = vec4(diffuse + specular + ambient, 1);
}
