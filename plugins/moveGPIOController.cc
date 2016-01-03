#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "../thirdparty/gpioController.h"
#include <cmath>

struct moveGPIOController : public FleyePlugin
{
	void setup(FleyeContext* ctx)
	{
		init_gpio();
	}

	void run(FleyeContext* ctx,int threadId)
	{
		 float theta = std::sin( ctx->frameCounter*0.02 );
		 float phi = std::cos( ctx->frameCounter*0.03 );
		 gpio_write_theta_phi(theta,phi,0);
	}

};

FLEYE_REGISTER_PLUGIN(moveGPIOController);
