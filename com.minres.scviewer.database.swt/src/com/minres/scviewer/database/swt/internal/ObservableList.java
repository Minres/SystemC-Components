/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database.swt.internal;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

public class ObservableList<E> implements List<E> {
	private List<E> delegate;
	private PropertyChangeSupport pcs;
	public static final String SIZE_PROPERTY = "size";
	public static final String CONTENT_PROPERTY = "content";

	public ObservableList() {
		this(new ArrayList<E>());
	}

	public ObservableList(List<E> delegate) {
		this.delegate = delegate;
		this.pcs = new PropertyChangeSupport(this);
	}

	public List<E> getContent() {
		return Collections.unmodifiableList(this.delegate);
	}

	protected List<E> getDelegateList() {
		return this.delegate;
	}

	protected void fireElementAddedEvent(int index, Object element) {
		fireElementEvent(new ElementAddedEvent(this, element, index));
	}

	protected void fireMultiElementAddedEvent(int index, List<E> values) {
		fireElementEvent(new MultiElementAddedEvent(this, index, values));
	}

	protected void fireElementClearedEvent(List<E> values) {
		fireElementEvent(new ElementClearedEvent(this, values));
	}

	protected void fireElementRemovedEvent(int index, Object element) {
		fireElementEvent(new ElementRemovedEvent(this, element, index));
	}

	protected void fireMultiElementRemovedEvent(List<E> values) {
		fireElementEvent(new MultiElementRemovedEvent(this, values));
	}

	protected void fireElementUpdatedEvent(int index, Object oldValue, Object newValue) {
		fireElementEvent(new ElementUpdatedEvent(this, oldValue, newValue, index));
	}

	protected void fireElementEvent(ElementEvent event) {
		this.pcs.firePropertyChange(event);
	}

	protected void fireSizeChangedEvent(int oldValue, int newValue) {
		this.pcs.firePropertyChange(new PropertyChangeEvent(this, "size", Integer.valueOf(oldValue), Integer
				.valueOf(newValue)));
	}

	public void add(int index, E element) {
		int oldSize = size();
		this.delegate.add(index, element);
		fireElementAddedEvent(index, element);
		fireSizeChangedEvent(oldSize, size());
	}

	public boolean add(E o) {
		int oldSize = size();
		boolean success = this.delegate.add(o);
		if (success) {
			fireElementAddedEvent(size() - 1, o);
			fireSizeChangedEvent(oldSize, size());
		}
		return success;
	}

	public boolean addAll(Collection<? extends E> c) {
		int oldSize = size();
		int index = size() - 1;
		index = (index < 0) ? 0 : index;

		boolean success = this.delegate.addAll(c);
		if ((success) && (c != null)) {
			List<E> values = new ArrayList<E>();
			for (Iterator<? extends E> i = c.iterator(); i.hasNext();) {
				values.add(i.next());
			}
			if (values.size() > 0) {
				fireMultiElementAddedEvent(index, values);
				fireSizeChangedEvent(oldSize, size());
			}
		}

		return success;
	}

	public boolean addAll(int index, Collection<? extends E> c) {
		int oldSize = size();
		boolean success = this.delegate.addAll(index, c);

		if ((success) && (c != null)) {
			List<E> values = new ArrayList<E>();
			for (Iterator<? extends E> i = c.iterator(); i.hasNext();) {
				values.add(i.next());
			}
			if (values.size() > 0) {
				fireMultiElementAddedEvent(index, values);
				fireSizeChangedEvent(oldSize, size());
			}
		}

		return success;
	}

	public void clear() {
		int oldSize = size();
		List<E> values = new ArrayList<E>();
		values.addAll(this.delegate);
		this.delegate.clear();
		if (!(values.isEmpty())) {
			fireElementClearedEvent(values);
		}
		fireSizeChangedEvent(oldSize, size());
	}

	public boolean contains(Object o) {
		return this.delegate.contains(o);
	}

	public boolean containsAll(Collection<?> c) {
		return this.delegate.containsAll(c);
	}

	public boolean equals(Object o) {
		return this.delegate.equals(o);
	}

	public E get(int index) {
		return this.delegate.get(index);
	}

	public int hashCode() {
		return this.delegate.hashCode();
	}

	public int indexOf(Object o) {
		return this.delegate.indexOf(o);
	}

	public boolean isEmpty() {
		return this.delegate.isEmpty();
	}

	public Iterator<E> iterator() {
		return new ObservableIterator(this.delegate.iterator());
	}

	public int lastIndexOf(Object o) {
		return this.delegate.lastIndexOf(o);
	}

	public ListIterator<E> listIterator() {
		return new ObservableListIterator(this.delegate.listIterator(), 0);
	}

	public ListIterator<E> listIterator(int index) {
		return new ObservableListIterator(this.delegate.listIterator(index), index);
	}

	public E remove(int index) {
		int oldSize = size();
		E element = this.delegate.remove(index);
		fireElementRemovedEvent(index, element);
		fireSizeChangedEvent(oldSize, size());
		return element;
	}

	public boolean remove(Object o) {
		int index = this.delegate.indexOf(o);
		if(index<0) return false;
		return remove(index)!=null;
	}

	public boolean remove(Collection<?> o) {
		int oldSize = size();
		int index = this.delegate.indexOf(o);
		boolean success = this.delegate.remove(o);
		if (success) {
			fireElementRemovedEvent(index, o);
			fireSizeChangedEvent(oldSize, size());
		}
		return success;
	}
	public boolean removeAll(Collection<?> c) {
		if (c == null) {
			return false;
		}

		List<E> values = new ArrayList<E>();
		if (c != null) {
			for (Iterator<?> i = c.iterator(); i.hasNext();) {
				@SuppressWarnings("unchecked")
				E element = (E) i.next();
				if (this.delegate.contains(element)) {
					values.add(element);
				}
			}
		}

		int oldSize = size();
		boolean success = this.delegate.removeAll(c);
		if ((success) && (!(values.isEmpty()))) {
			fireMultiElementRemovedEvent(values);
			fireSizeChangedEvent(oldSize, size());
		}

		return success;
	}

	public boolean retainAll(Collection<?> c) {
		if (c == null) {
			return false;
		}

		List<E> values = new ArrayList<E>();
		Iterator<? extends E> i;
		if (c != null) {
			for (i = this.delegate.iterator(); i.hasNext();) {
				E element = i.next();
				if (!(c.contains(element))) {
					values.add(element);
				}
			}
		}

		int oldSize = size();
		boolean success = this.delegate.retainAll(c);
		if ((success) && (!(values.isEmpty()))) {
			fireMultiElementRemovedEvent(values);
			fireSizeChangedEvent(oldSize, size());
		}

		return success;
	}

	public E set(int index, E o) {
		E oldValue = this.delegate.set(index, o);
		fireElementUpdatedEvent(index, oldValue, o);
		return oldValue;
	}

	public int size() {
		return this.delegate.size();
	}

	public int getSize() {
		return size();
	}

	public List<E> subList(int fromIndex, int toIndex) {
	    return this.delegate.subList(fromIndex, toIndex);
	}

	public void rotate(int fromIndex, int toIndex, int distance){
	    Collections.rotate(this.delegate.subList(fromIndex, toIndex), distance);
        fireElementEvent(new MultiElementUpdatedEvent(this, this.delegate.subList(fromIndex, toIndex)));
	}
	
	public Object[] toArray() {
		return this.delegate.toArray();
	}

    public <T> T[] toArray(T[] a){
		return this.delegate.toArray(a);
	}

	public void addPropertyChangeListener(PropertyChangeListener listener) {
		this.pcs.addPropertyChangeListener(listener);
	}

	public void addPropertyChangeListener(String propertyName, PropertyChangeListener listener) {
		this.pcs.addPropertyChangeListener(propertyName, listener);
	}

	public PropertyChangeListener[] getPropertyChangeListeners() {
		return this.pcs.getPropertyChangeListeners();
	}

	public PropertyChangeListener[] getPropertyChangeListeners(String propertyName) {
		return this.pcs.getPropertyChangeListeners(propertyName);
	}

	public void removePropertyChangeListener(PropertyChangeListener listener) {
		this.pcs.removePropertyChangeListener(listener);
	}

	public void removePropertyChangeListener(String propertyName, PropertyChangeListener listener) {
		this.pcs.removePropertyChangeListener(propertyName, listener);
	}

	public boolean hasListeners(String propertyName) {
		return this.pcs.hasListeners(propertyName);
	}

    public static class MultiElementUpdatedEvent extends ObservableList.ElementEvent {
        /**
         * 
         */
        private static final long serialVersionUID = 7819626246672640599L;
        
        private List<Object> values = new ArrayList<Object>();

        public MultiElementUpdatedEvent(Object source, List<?> values) {
            super(source, ObservableList.ChangeType.oldValue, ObservableList.ChangeType.newValue, 0,
                    ObservableList.ChangeType.MULTI_UPDATED);
            if (values != null)
                this.values.addAll(values);
        }

        public List<?> getValues() {
            return Collections.unmodifiableList(this.values);
        }
    }

	public static class MultiElementRemovedEvent extends ObservableList.ElementEvent {
		/**
		 * 
		 */
		private static final long serialVersionUID = 7819626246672640599L;
		
		private List<Object> values = new ArrayList<Object>();

		public MultiElementRemovedEvent(Object source, List<?> values) {
			super(source, ObservableList.ChangeType.oldValue, ObservableList.ChangeType.newValue, 0,
					ObservableList.ChangeType.MULTI_REMOVE);
			if (values != null)
				this.values.addAll(values);
		}

		public List<?> getValues() {
			return Collections.unmodifiableList(this.values);
		}
	}

	public static class MultiElementAddedEvent extends ObservableList.ElementEvent {
		/**
		 * 
		 */
		private static final long serialVersionUID = -116376519087713082L;
		private List<Object> values = new ArrayList<Object>();

		public MultiElementAddedEvent(Object source, int index, List<?> values) {
			super(source, ObservableList.ChangeType.oldValue, ObservableList.ChangeType.newValue, index,
					ObservableList.ChangeType.MULTI_ADD);
			if (values != null)
				this.values.addAll(values);
		}

		public List<?> getValues() {
			return Collections.unmodifiableList(this.values);
		}
	}

	public static class ElementClearedEvent extends ObservableList.ElementEvent {
		/**
		 * 
		 */
		private static final long serialVersionUID = -8654027608903811577L;
		private List<Object> values = new ArrayList<Object>();

		public ElementClearedEvent(Object source, List<?> values) {
			super(source, ObservableList.ChangeType.oldValue, ObservableList.ChangeType.newValue, 0,
					ObservableList.ChangeType.CLEARED);
			if (values != null)
				this.values.addAll(values);
		}

		public List<?> getValues() {
			return Collections.unmodifiableList(this.values);
		}
	}

	public static class ElementRemovedEvent extends ObservableList.ElementEvent {
		/**
		 * 
		 */
		private static final long serialVersionUID = -6664217547528652003L;

		public ElementRemovedEvent(Object source, Object value, int index) {
			super(source, value, null, index, ObservableList.ChangeType.REMOVED);
		}
	}

	public static class ElementUpdatedEvent extends ObservableList.ElementEvent {
		/**
		 * 
		 */
		private static final long serialVersionUID = 7793549621724991011L;

		public ElementUpdatedEvent(Object source, Object oldValue, Object newValue, int index) {
			super(source, oldValue, newValue, index, ObservableList.ChangeType.UPDATED);
		}
	}

	public static class ElementAddedEvent extends ObservableList.ElementEvent {
		/**
		 * 
		 */
		private static final long serialVersionUID = -6990071468319043554L;

		public ElementAddedEvent(Object source, Object newValue, int index) {
			super(source, null, newValue, index, ObservableList.ChangeType.ADDED);
		}
	}

	public static abstract class ElementEvent extends PropertyChangeEvent {
		/**
		 * 
		 */
		private static final long serialVersionUID = 964946867437728530L;
		private final ObservableList.ChangeType type;
		private final int index;

		public ElementEvent(Object source, Object oldValue, Object newValue, int index, ObservableList.ChangeType type) {
			super(source, "content", oldValue, newValue);
			this.type = type;
			this.index = index;
		}

		public int getIndex() {
			return this.index;
		}

		public int getType() {
			return this.type.ordinal();
		}

		public ObservableList.ChangeType getChangeType() {
			return this.type;
		}

		public String getTypeAsString() {
			return this.type.name().toUpperCase();
		}
	}

	public static enum ChangeType {
		ADDED, UPDATED, REMOVED, CLEARED, MULTI_ADD, MULTI_UPDATED, MULTI_REMOVE, NONE;

		public static final Object oldValue;
		public static final Object newValue;

		public static ChangeType resolve(int ordinal) {
			switch (ordinal) {
			case 0:
				return ADDED;
			case 2:
				return REMOVED;
			case 3:
				return CLEARED;
			case 4:
				return MULTI_ADD;
			case 5:
				return MULTI_REMOVE;
			case 6:
				return NONE;
			case 1:
			}
			return UPDATED;
		}

		static {
			oldValue = new Object();
			newValue = new Object();
		}
	}

	protected class ObservableListIterator extends ObservableList<E>.ObservableIterator implements ListIterator<E> {

		public ObservableListIterator(ListIterator<E> listIterator, int index) {
			super(listIterator);
			this.cursor = (index - 1);
		}

		public ListIterator<E> getListIterator() {
			return ((ListIterator<E>) getDelegate());
		}

		public void add(E o) {
			ObservableList.this.add(o);
			this.cursor += 1;
		}

		public boolean hasPrevious() {
			return getListIterator().hasPrevious();
		}

		public int nextIndex() {
			return getListIterator().nextIndex();
		}

		public E previous() {
			return getListIterator().previous();
		}

		public int previousIndex() {
			return getListIterator().previousIndex();
		}

		public void set(E o) {
			ObservableList.this.set(this.cursor, o);
		}
	}

	protected class ObservableIterator implements Iterator<E> {

		private Iterator<E> iterDelegate;

		protected int cursor = -1;

		public ObservableIterator(Iterator<E> paramIterator) {
			this.iterDelegate = paramIterator;
		}

		public Iterator<E> getDelegate() {
			return this.iterDelegate;
		}

		public boolean hasNext() {
			return this.iterDelegate.hasNext();
		}

		public E next() {
			this.cursor += 1;
			return this.iterDelegate.next();
		}

		public void remove() {
			int oldSize = ObservableList.this.size();
			Object element = ObservableList.this.get(this.cursor);
			this.iterDelegate.remove();
			ObservableList.this.fireElementRemovedEvent(this.cursor, element);
			ObservableList.this.fireSizeChangedEvent(oldSize, ObservableList.this.size());
			this.cursor -= 1;
		}
	}
}