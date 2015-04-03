attribute vec4 aPosition;
attribute vec3 aNormal;
uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
varying vec4 normal;
varying vec4 vertex;

void main()
{
	gl_Position = uProjMatrix * uViewMatrix * uModelMatrix * aPosition;
	normal = normalize(uModelMatrix * vec4(aNormal, 0));
	vertex = uModelMatrix * aPosition;
}
