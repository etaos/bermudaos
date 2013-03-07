/*
 *  BermudaOS - Thread header
 *  Copyright (C) 2012   Michel Megens
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __THREAD_H_
#define __THREAD_H_

#include <lib/binary.h>

#include <kernel/event.h>
#include <kernel/stack.h>

#define THREAD_SLEEPING_SHIFT 0
#define THREAD_RUNNING_SHIFT 1
#define THREAD_READY_SHIFT 2
#define THREAD_KILLED_SHIFT 3

#define THREAD_SLEEPING_MASK BIT(THREAD_SLEEPING_SHIFT)
#define THREAD_RUNNING_MASK BIT(THREAD_RUNNING_SHIFT)
#define THREAD_READY_MASK BIT(THREAD_READY_SHIFT)
#define THREAD_KILLED_MASK BIT(THREAD_KILLED_SHIFT)

/**
 * \brief Thread handle type definition.
 * \param param Thread parameter.
 */
typedef void (*thread_handle_t)(void *param);

typedef enum 
{
	THREAD_RED,
	THREAD_BLACK,
} thread_color_t;

struct thread_root
{
	struct thread *tree;
	
	size_t size;
} __attribute__((packed));

/**
 * \brief Thread structure.
 * 
 * This structure defines the current state of a thread.
 */
struct thread
{
	struct thread *left; //!< Left pointer in the red black tree.
	struct thread *right; //!< Right pointer in the red black tree.
	struct thread *parent;
	struct thread *next; //!< Next queue pointer.
	struct thread *volatile*queue; //!< Pointer pointer to the current queue.
	uint64_t cpu_time; //!< Amount of time (in ms) this thread has been using the CPU.
	thread_color_t color; //!< Node color. \see thread_color_t
	
	char *name; //!< Unique thread name.
	uint64_t id; //!< Unique thread identifier.
	
	struct event *event; //!< Event attached to this thread.
	struct stack *stack; //!< Stack structure.
	uint8_t ec; //!< Event counter.
	uint16_t sleep_time; //!< Time to sleep left.
	uint8_t priority; //!< Priority of this thread.
	thread_handle_t handle; //!< The thread handle.
	void *arg; //!< Argument past to the thread.
	
	/* current state */
	uint8_t sleeping : 1; //!< If set to one, this thread is sleeping.
	uint8_t running : 1; //!< If set to one, this thread is running.
	uint8_t ready : 1; //!< If set to one, this thread is ready to run.
	uint8_t killed : 1; //!< If set to one, this thread is about to be killed.
	uint8_t signaled : 1; //!< This thread received an event signal.
	uint8_t irq_signaled : 1; //!< This thread received a signal from an IRQ handle.
} __attribute__((packed));

__DECL
static inline struct thread *thread_parent(struct thread *node)
{
	if(node) {
		return node->parent;
	} else {
		return NULL;
	}
}

static inline struct thread *thread_grandparent(struct thread *node)
{
	struct thread *parent;
	if((parent = thread_parent(node)) != NULL) {
		return thread_parent(parent);
	} else {
		return NULL;
	}
}

/**
 * \brief Check wether the given node has a sibling.
 * \param node Node to check.
 * \retval NULL if \p node has no sibling.
 * \return The sibling of \p node.
 */
static inline struct thread *thread_node_has_sibling(struct thread *node)
{
	struct thread *parent;
	if(node) {
		parent = thread_parent(node);
		if(parent) {
			if(node == parent->left) {
				/* node is on the left of parent, so the sibling of node is on right of parent. */
				return parent->right;
			} else {
				return parent->left;
			}
		}
	}
	
	return NULL;
}

/**
 * \brief Return the 'far nephew' of the given node.
 * \param node The 'far nephew' of \p node will be returned.
 * \retval NULL if there is no 'far newphew'.
 */
static inline struct thread *thread_node_far_nephew(struct thread *node)
{
	struct thread *parent;
	
	if(node && thread_node_has_sibling(node)) {
		parent = thread_parent(node);
		return (parent->left == node) ? parent->right->right : parent->left->left;
	}
	return NULL;
}

static inline int thread_parent_on_left(struct thread *current)
{
	if(current->parent != NULL && current->parent->right == current) {
		return 1;
	}
	return 0;
}

extern int thread_add_new(struct thread *t, void *stack, size_t stack_size);
extern int thread_core_init(void *mstack, size_t mstack_size);
extern int thread_insert(struct thread_root *root, struct thread *node);
extern struct thread *thread_search(struct thread_root *root, int key);
extern int thread_delete_node(struct thread_root *root, struct thread *node);
#ifdef HAVE_SCHED_DBG
extern void thread_dump(struct thread *tree, FILE *stream);
extern void thread_add_node(struct thread_root *root, int key);
extern void thread_cleanup(struct thread *root);
#endif /* HAVE_SCHED_DBG */
__DECL_END
#endif
