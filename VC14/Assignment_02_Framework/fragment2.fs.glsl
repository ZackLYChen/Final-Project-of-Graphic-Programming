#version 410 core
#define KERNEL_SIZE 9
#define T texture2D(tex,.5+(p.xy*=.992))
uniform sampler2D tex;

uniform vec2 img_size;
uniform int modes;
uniform float time;
uniform vec2 mouse;

const float PI = 3.1415926535;
in vec2 uv;
layout(location = 0)out vec4 fragColor;


in VS_OUT {
	vec2 texcoord;
} fs_in;

void main(void) { 
	if(modes==0){
		vec4 Color  = texture(tex,fs_in.texcoord);
		fragColor = Color;
	}
	else if(modes==1){
		vec4 Left_Color  = texture(tex,fs_in.texcoord+vec2(0.01, 0));
		vec4 Right_Color = texture(tex,fs_in.texcoord+vec2(-0.01, 0));
		float r = Left_Color.r*0.299+Left_Color.g*0.587+Left_Color.b*0.114;
		float g = Right_Color.g;
		float b = Right_Color.b;
		fragColor= vec4(r, g, b, 1.0);
	}
	else if (modes == 2){            
	   vec4 sum = vec4(0);
	   vec2 texcoord = fs_in.texcoord;
  
	   for( int i= -4 ;i < 4; i++)
	   {
			for ( int j = -3; j < 3; j++)
			{
				sum += texture2D(tex, texcoord + vec2(j, i)*0.004) * 0.25;
			}
	   }
		   if (texture2D(tex, texcoord).r < 0.3)
		{
		   fragColor = sum*sum*0.012 + texture2D(tex, texcoord);
		}
		else
		{
			if (texture2D(tex, texcoord).r < 0.5)
			{
				fragColor = sum*sum*0.009 + texture2D(tex, texcoord);
			}
			else
			{
				fragColor = sum*sum*0.0075 + texture2D(tex, texcoord);
			}
		}
	}
	else if(modes==3){
		float sigma_e = 2.0f; 
		float sigma_r = 2.8f; 
		float phi = 3.4f; 
		float tau = 0.99f;

		float twoSigmaESquared = 2.0 * sigma_e * sigma_e;
		float twoSigmaRSquared = 2.0 * sigma_r * sigma_r;
		int halfWidth = int(ceil( 2.0 * sigma_r ));

		vec4 color1 = vec4(0); 
		int n =0; 
		int half_size = 2; 
		for ( int i = - half_size; i <= half_size ; ++i ) { 
			for ( int j = - half_size; j <= half_size ; ++j ) { 
				vec4 c= texture(tex,fs_in.texcoord +vec2(i,j)/img_size); 
				color1+= c; 
				n++;
			} 
		} 
		color1 /=n; 
		
		vec4 color2 = vec4(0); 
		int nbins = 8; 
		float r = floor(color1.r * float(nbins)) / float(nbins);
		float g = floor(color1.g * float(nbins)) / float(nbins); 
		float b = floor(color1.b * float(nbins)) / float(nbins);
		color2 = vec4(r,g,b,color1.a); 

		vec4 color3 = vec4(0); 
		vec2 sum = vec2(0.0); 
		vec2 norm = vec2(0.0); 
		for ( int i = -halfWidth; i <= halfWidth; ++i ) {
			for ( int j = -halfWidth; j <= halfWidth; ++j ) { 
				float d = length(vec2(i,j)); 
				vec2 kernel= vec2( exp( -d * d / twoSigmaESquared ), exp( -d * d / twoSigmaRSquared )); 
				vec4 c= texture(tex,fs_in.texcoord+vec2(i,j)/img_size);
				vec2 L= vec2(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
				norm += 2.0 * kernel; 
				sum += kernel * L;
			} 
		}
		sum /= norm; 
		float H = 100.0 * (sum.x - tau * sum.y); 
		float edge =( H > 0.0 )?1.0:2.0 *smoothstep(-2.0, 2.0, phi * H ); 
		color3 = vec4(edge,edge,edge,1.0 );

		fragColor= color2*color3;
	}
	else if (modes==4){
		vec2 p = fs_in.texcoord; 
		float A =time;
		float w = 10.0 * PI;
		float t = 30.0*PI/180.0;
		float y = sin( w*p.x + t) * A; 
		vec2 uv= vec2(p.x, p.y+time*y);
		vec4 tcolor = texture2D(tex, uv); 
		fragColor = tcolor; 
	}
	else if (modes == 5){
		  float aperture = 178.0;
		  float apertureHalf = 0.5 * aperture * (PI / 180.0);
		  float maxFactor = sin(apertureHalf);
  
		  vec2 uv;
		  vec2 xy = 2.0 * fs_in.texcoord - 1.0;
		  float d = length(xy);
		  if (d < (2.0-maxFactor))
		  {
			d = length(xy * maxFactor);
			float z = sqrt(1.0 - d * d);
			float r = atan(d, z) / PI;
			float phi = atan(xy.y, xy.x);
    
			uv.x = r * cos(phi) + 0.5;
			uv.y = r * sin(phi) + 0.5;
		  }
		  else
		  {
			uv = fs_in.texcoord;
		  }
		  vec4 c = texture2D(tex, uv);
		  fragColor = c;
	}
	else if(modes == 6){
		float kernel[KERNEL_SIZE];
		float step_w = 1.0/img_size.x;
		float step_h = 1.0/img_size.y;
	
		vec2 offset[KERNEL_SIZE];

	   int i = 0;
	   vec4 sum = vec4(0.0);
	   
	   offset[0] = vec2(-step_w, -step_h);
	   offset[1] = vec2(0.0, -step_h);
	   offset[2] = vec2(step_w, -step_h);
	   
	   offset[3] = vec2(-step_w, 0.0);
	   offset[4] = vec2(0.0, 0.0);
	   offset[5] = vec2(step_w, 0.0);
	   
	   offset[6] = vec2(-step_w, step_h);
	   offset[7] = vec2(0.0, step_h);
	   offset[8] = vec2(step_w, step_h);
	   
	   kernel[0] = -1.0; 	kernel[1] = -1.0;	kernel[2] = -1.0;
	   kernel[3] = -1.0;	kernel[4] = 9.0;	kernel[5] = -1.0;
	   kernel[6] = -1.0;   	kernel[7] = -1.0;	kernel[8] = -1.0;
	   
		for( i=0; i<KERNEL_SIZE; i++ )
		{
			vec4 tmp = texture2D(tex, fs_in.texcoord + offset[i]);
			sum += tmp * kernel[i];
		}

	   fragColor = sum;
	}
	else if (modes==7){
		float kernel[KERNEL_SIZE] ;
		 kernel[0]=0.0;
		 kernel[1]=1.0;
		 kernel[2]=0.0;
		 kernel[3]=1.0;
		 kernel[4]=-4.0;
		 kernel[5]=1.0;
		 kernel[6]=0.0;
		 kernel[7]=1.0;
		 kernel[8]=0.0;
	
		float step_w = 1.0/img_size.x;
		float step_h = 1.0/img_size.y;
	
		vec2 offset[KERNEL_SIZE];
		 offset[0] =vec2(-step_w, -step_h);
		 offset[1] =vec2(0.0, -step_h);
		 offset[2] =vec2(step_w, -step_h);
		 offset[3] = vec2(-step_w, 0.0);
		 offset[4] =vec2(0.0, 0.0);
		 offset[5] = vec2(step_w, 0.0);
		 offset[6] = vec2(-step_w, step_h);
		 offset[7] = vec2(0.0, step_h);
		 offset[8] =vec2(step_w, step_h) ;

		int i = 0;
		vec4 sum = vec4(0.0);
		for( i=0; i<KERNEL_SIZE; i++ )
		{
			vec4 tmp = texture2D(tex, fs_in.texcoord + offset[i]);
			sum += tmp * kernel[i];
		}
		 //vec4 base_color = texture2D(tex, fs_in.texcoord);
		 //sum = 0.5*sum + base_color;
		 float r = sum.r*0.299+sum.g*0.587+sum.b*0.114;
		 float g = sum.r*0.299+sum.g*0.587+sum.b*0.114;
		 float b = sum.r*0.299+sum.g*0.587+sum.b*0.114;
		 fragColor= vec4(r, g, b, 1.0);
	}
	else if (modes == 8){
		float R = 100.0;
		float h = 40.0;
		float hr = R * sqrt(1.0 - ((R - h) / R) * ((R - h) / R));

		vec2 xy = gl_FragCoord.xy - mouse.xy;
		float r = sqrt(xy.x * xy.x + xy.y * xy.y);
		vec2 new_xy = r < hr ? xy  / 5 : xy;
	
		fragColor = texture2D(tex, (new_xy + mouse)/img_size);
	}
	else if (modes == 9){
			float sampleDist = 1.0;
			float sampleStrength = 2.2; 
		    float samples[10];
			samples[0] = -0.08;
			samples[1] = -0.05;
			samples[2] = -0.03;
			samples[3] = -0.02;
			samples[4] = -0.01;
			samples[5] =  0.01;
			samples[6] =  0.02;
			samples[7] =  0.03;
			samples[8] =  0.05;
			samples[9] =  0.08;

			vec2 dir = 0.5 - uv; 
			float dist = sqrt(dir.x*dir.x + dir.y*dir.y); 
			dir = dir/dist; 

			vec4 color = texture2D(tex,uv); 
			vec4 sum = color;

			for (int i = 0; i < 10; i++)
				sum += texture2D( tex, uv + dir * samples[i] * sampleDist );

			sum *= 1.0/11.0;
			float t = dist * sampleStrength;
			t = clamp( t ,0.0,1.0);

			fragColor = mix( color, sum, t );
	}
}


