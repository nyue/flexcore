/*
 * parallelregion.h
 *
 *  Created on: Oct 5, 2015
 *      Author: ckielwein
 */

#ifndef SRC_THREADING_PARALLELREGION_HPP_
#define SRC_THREADING_PARALLELREGION_HPP_

#include <ports/event_ports.hpp>

namespace fc
{

class region_id
{
};

bool operator==(const region_id& lhs, const region_id& rhs);

class region_info
{
	virtual region_id get_id() const = 0;
	virtual event_out_port<void> switch_tick() = 0;
	virtual event_out_port<void> work_tick() = 0;

protected:
	//destructor is not public, as no ownership to regions is given through this.
	~region_info() = default;
};


class tick_controller
{
public:
	tick_controller() = default;

	/// sends void event on the switch tick of the surrounding region
	event_out_port<void> switch_tick() { return switch_buffers; }
	/**
	 * \brief  sends void event on the work tick of the surrounding region
	 * connect nodes, that want to be triggered every cycle to this.
	 */
	event_out_port<void> work_tick() { return work; }

	//todo add real functionality instead of only forwarding for example subsampling of work tick to get lower cycling rate.
	auto in_switch_buffers() { return [this](){return switch_buffers.fire(); };}
	auto in_work() { return [this](){return work.fire(); };}

	event_out_port<void> switch_buffers;
	event_out_port<void> work;
};

class parallel_region : public region_info
{
public:
	virtual ~parallel_region() = default;
	parallel_region() = default;

	region_id get_id() const override;
	event_out_port<void> switch_tick() override;
	event_out_port<void> work_tick() override;
protected:
	//copy constructor is protected to avoid slicing, since this is meant as base class
	parallel_region(const parallel_region&) = default;
	parallel_region& operator=(const parallel_region&) = default;
private:
	tick_controller ticks;
	region_id id;

};

} /* namespace fc */

#endif /* SRC_THREADING_PARALLELREGION_HPP_ */
