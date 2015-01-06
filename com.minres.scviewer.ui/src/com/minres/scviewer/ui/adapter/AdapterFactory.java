package com.minres.scviewer.ui.adapter;

import org.eclipse.core.runtime.IAdapterFactory;
import org.eclipse.ui.views.properties.IPropertySource;

import com.minres.scviewer.database.ITx;

public class AdapterFactory implements IAdapterFactory {

	@SuppressWarnings("rawtypes")
	@Override
	public Object getAdapter(Object adaptableObject, Class adapterType) {
		if (adapterType == IPropertySource.class)
			return new ITransactionPropertySource((ITx) adaptableObject);
		return null;
	}

	@SuppressWarnings("rawtypes")
	@Override
	public Class[] getAdapterList() {
		return new Class[]{IPropertySource.class};
	}

}
