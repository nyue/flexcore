#ifndef SRC_PORTS_REGION_AWARE_HPP_
#define SRC_PORTS_REGION_AWARE_HPP_

#include <ports/port_traits.hpp>
#include <core/connection.hpp>
#include <threading/parallelregion.hpp>

namespace fc
{

/**
 * \brief A mixin for ports that are aware of the region, they belong to.
 */
template<class port_t>
struct region_aware: public port_t
{
	static_assert(is_port<port_t>::value,
			"node_aware_port can only be mixed to port types");
	//allows explicit access to base of this mixin.
	typedef port_t base_t;

	template<class ... args>
	region_aware(std::shared_ptr<region_info> region,
	        args ... base_constructor_args) :
			port_t(base_constructor_args...), parent_region_info(region)
	{

	}

	//need weak_ptr here as we have a cycle: region -> node -> port -> region
	std::weak_ptr<region_info> parent_region_info;
};

template<class source_t, class sink_t>
bool same_region(const source_t& source, const sink_t& sink)
{
	return source.parent_region_info.lock()->get_id()
	        == sink.parent_region_info.lock()->get_id();
}

class buffer_interface;
class buffer;
class no_buffer;

template<class source_t, class sink_t>
std::shared_ptr<buffer_interface> construct_buffer(const source_t& source, const sink_t& sink)
{
	if (same_region(source, sink))
	{
		auto result_buffer = std::make_shared<buffer>();
		source.parent_region_info.lock()->switch_tick() >>
				result_buffer->in_switch();
		sink.parent_region_info.lock()->work_tick() >>
				result_buffer->in_send();
		return result;
	}
	else
		return std::make_shared<no_buffer>();
}

template<class base_connection>
struct node_aware_connection: public base_connection
{
	node_aware_connection(std::shared_ptr<buffer_interface> new_buffer,
	        const base_connection& base) :
			base_connection(base),
			buffer(new_buffer)
	{
	}

	bool already_buffered = false;
	std::shared_ptr<buffer_interface> buffer;
private:

};

template<class base_connection>
auto wrap_node_aware(const base_connection& base)
{
	return node_aware_connection<base_connection>(base);
}

// TODO prefer to test this algorithmically
template<class T> struct is_port<region_aware<T>> : public std::true_type
{
};

namespace detail
{
template<class source_t, class sink_t>
struct node_aware_connect_impl
{
	auto operator()(source_t source, sink_t sink)
	{
		const auto base = static_cast<typename source_t::base_t>(source);
		return wrap_node_aware(::fc::connect(base, sink));
	}
};

}  //namespace detail

template<class source_t, class sink_t>
auto connect(region_aware<source_t> source, sink_t sink)
{
	return detail::node_aware_connect_impl<region_aware<source_t>, sink_t>()(
	        source, sink);
}

template<class source_t, class sink_t>
auto connect(region_aware<source_t> source, region_aware<sink_t> sink)
{
	//construct node_aware_connection
	//based on if source and sink are from same region
	return detail::node_aware_connect_impl<region_aware<source_t>,
	        region_aware<sink_t>>()(construct_buffer(source, sink),
	        source, sink);
}

template<class base_connection, class sink_t>
auto connect(node_aware_connection<base_connection> source, sink_t sink)
{
	return wrap_node_aware(connect(static_cast<base_connection>(source), sink));
}

}  //namespace fc

#endif /* SRC_PORTS_REGION_AWARE_HPP_ */
