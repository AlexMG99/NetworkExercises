#include "Networks.h"


#if defined(USE_TASK_MANAGER)

void ModuleResources::TaskLoadTexture::execute()
{
	*texture = App->modTextures->loadTexture(filename);
}

#endif


bool ModuleResources::init()
{
	background = App->modTextures->loadTexture("background.jpg");

#if !defined(USE_TASK_MANAGER)
	space = App->modTextures->loadTexture("space_background.jpg");
	asteroid1 = App->modTextures->loadTexture("asteroid1.png");
	asteroid2 = App->modTextures->loadTexture("asteroid2.png");
	spacecraft1 = App->modTextures->loadTexture("spaceship_01.png");
	spacecraft2 = App->modTextures->loadTexture("spaceship_02.png");
	spacecraft3 = App->modTextures->loadTexture("spacecraft3.png");
	start = App->modTextures->loadTexture("start.png");
	lost = App->modTextures->loadTexture("lost.png");
	loadingFinished = true;
	completionRatio = 1.0f;
#else
	loadTextureAsync("space_background.jpg", &space);
	loadTextureAsync("asteroid1.png",        &asteroid1);
	loadTextureAsync("asteroid2.png",        &asteroid2);
	loadTextureAsync("spaceship_01.png",      &spacecraft1);
	loadTextureAsync("spaceship_02.png",      &spacecraft2);
	loadTextureAsync("spacecraft3.png",      &spacecraft3);
	loadTextureAsync("laser_1.png",            &laser);
	loadTextureAsync("explosion1.png",       &explosion1);
	loadTextureAsync("start.png",				&start);
	loadTextureAsync("lost.png",				&lost);
#endif

	audioClipLaser = App->modSound->loadAudioClip("laser.wav");
	audioClipExplosion = App->modSound->loadAudioClip("explosion.wav");
	//App->modSound->playAudioClip(audioClipExplosion);

	return true;
}

#if defined(USE_TASK_MANAGER)

void ModuleResources::loadTextureAsync(const char * filename, Texture **texturePtrAddress)
{
	ASSERT(taskCount < MAX_RESOURCES);
	
	TaskLoadTexture *task = &tasks[taskCount++];
	task->owner = this;
	task->filename = filename;
	task->texture = texturePtrAddress;

	App->modTaskManager->scheduleTask(task, this);
}

void ModuleResources::onTaskFinished(Task * task)
{
	ASSERT(task != nullptr);

	TaskLoadTexture *taskLoadTexture = dynamic_cast<TaskLoadTexture*>(task);

	for (uint32 i = 0; i < taskCount; ++i)
	{
		if (task == &tasks[i])
		{
			finishedTaskCount++;
			task = nullptr;
			break;
		}
	}

	ASSERT(task == nullptr);

	if (finishedTaskCount == taskCount)
	{
		finishedLoading = true;

		// Create the explosion animation clip
		explosionClip = App->modRender->addAnimationClip();
		explosionClip->frameTime = 0.1f;
		explosionClip->loop = false;
		for (int i = 0; i < 16; ++i)
		{
			float x = (i % 4) / 4.0f;
			float y = (i / 4) / 4.0f;
			float w = 1.0f / 4.0f;
			float h = 1.0f / 4.0f;
			explosionClip->addFrameRect(vec4{ x, y, w, h });
		}

		// Create the explosion animation clip
		spaceship01Clip = App->modRender->addAnimationClip();
		spaceship01Clip->frameTime = 0.02f;
		spaceship01Clip->loop = true;
		for (int i = 0; i < 4; ++i)
		{
			float x = i / 4.0f;
			float y = 0;
			float w = 1.0f / 4.0f;
			float h = 1.0f;
			spaceship01Clip->addFrameRect(vec4{ x, y, w, h });
		}

		spaceship02Clip = App->modRender->addAnimationClip();
		spaceship02Clip->frameTime = 0.1f;
		spaceship02Clip->loop = true;
		for (int i = 0; i < 2; ++i)
		{
			float x = i / 2.0f;
			float y = 0;
			float w = 1.0f / 2.0f;
			float h = 1.0f;
			spaceship02Clip->addFrameRect(vec4{ x, y, w, h });
		}
	}
}

#endif
