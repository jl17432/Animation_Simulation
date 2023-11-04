#include "Particle.h"

Particle::Particle()
{
	position = Vertex(0, 0, 0);
	velocity = Vertex(0, 0, 0);
	velocity_last_frame = Vertex(0, 0, 0);
	force = Vertex(0, 0, 0);
	density = 0.f;
	col_inGrid = 25 + floor(position.x);
	row_inGrid = floor(position.y);
}

Particle::Particle(Vertex &new_position)
{
	position = new_position;
	velocity = Vertex(0, 0, 0);
	velocity_last_frame = Vertex(0, 0, 0);
	force = Vertex(0, 0, 0);
	density = 0.f;
	col_inGrid = 25 + floor(new_position.x);
	row_inGrid = floor(new_position.y);
}


void Particle::updateVelocity(float delta_t)
{
	Vertex acc = force / 1.0f;
	velocity = velocity_last_frame + acc * delta_t;
	velocity_last_frame = velocity;
}

void Particle::updatePosition(float delta_t)
{
	position = position + velocity * delta_t;

	return;
}
