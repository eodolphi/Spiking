#include "tempeltonPeck.cc"
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <string>

int NOW = 0;


template< class LayerType> class Layer;
class Synapse;
class AddressLayer;
class AssociativeNeuron;
class Pattern;

class PotentialsQueue : public Queue<Double>
{
public:
	PotentialsQueue(string name)
    {
		fileName = name + ".potentials";
		stream = new ofstream(fileName.c_str(), ios::trunc);
    }

	~PotentialsQueue(void)
	{
		Item<Double> *time;
		
		for (time = first; time; time = time->next)
		{
			*stream << time->item->getValue() << "\n";
		}
		
        stream->close();
	} 
	string fileName;
	ofstream *stream;
};

class FiringTimesQueue : public Queue<Int>
{
public:
	FiringTimesQueue(string name)
    {
		fileName = name + ".firingTimes";
		stream = new ofstream(fileName.c_str(), ios::trunc);
		addNow();
    }
	~FiringTimesQueue(void);
  
	void addNow(void)
    {
		add( new Int(NOW + 1));
    }
	string fileName;
	ofstream *stream;
};

class FiringQueue : public Queue<Int>
{
public:
	FiringQueue(string name)
    {
		fileName = name + ".firings";
		stream = new ofstream(fileName.c_str(), ios::trunc);
    }

	~FiringQueue(void)
	{
		Item<Int> *time;
		for (time = first; time; time = time->next)
		{
			*stream << time->item->getValue() << "\n";
		}
		
        stream->close();
	} 
	string fileName;
	ofstream *stream;
};



class FiringRateQueue : public Queue<Double>
{
public:
	FiringRateQueue(FiringTimesQueue *times, string name, int window)
    {
		int time = 0;
		double rate = 0;
		Queue<Int> *past = new Queue<Int>;
      
      
		stream = new ofstream(name.c_str(), ios::trunc);
      

		while(time < NOW)
		{
			if(times->get() != NULL && times->get()->getValue() == time)
			{
				rate += 1 / (double) window;
				past->add(new Int(times->get()->getValue()));
				times->remove();
	      
			}
			if(past->get() != NULL && past->get()->getValue() == time - window )
			{
				rate -= 1 / (double) window;
				past->removeAndDelete();
			}

			if (time > 500)
				add(new Double(rate) );
			time++;
		}
    };

	double getAverage(void)
    {
		Item<Double> *temp;
		int nrOfItems = 0;
		double total = 0;
      
		for(temp = first; temp->next; temp = temp->next)
		{
			total += temp->item->getValue();
			nrOfItems++;
		}
		return(total / (double) nrOfItems);
    };

    ~FiringRateQueue(void)
    {
		Item<Double> *time;
		for (time = first; time; time = time->next)
		{
			*stream << time->item->getValue() << "\n";
		}
		stream->close();
    } 
    string fileName;
    ofstream *stream;
};


class AssociativeNeuronLogger
{
public:
	AssociativeNeuronLogger(AssociativeNeuron* n);
	~AssociativeNeuronLogger(void)
    {
		delete potentials;
		delete firingTimes;
		delete rates;
		delete firings;
	};

	void logPotential(void);
	void logFiring(void);

	void logFiringTime(void)
    {
		firingTimes->addNow();
    };

	void finalize(void);
    
	int num;
	ofstream *stream;
	AssociativeNeuron *neuron;
	FiringTimesQueue *firingTimes;
	PotentialsQueue *potentials;
	FiringQueue *firings;
	FiringRateQueue *rates;
};


class FiringTimes : public Queue<Int>
{
public:
	void addNow(void)
    {
		add(new Int(NOW));
		add(new Int(NOW));
    };
};



class Neuron
{
public:
	Neuron()
    {
		preSynapticSynapses = new Set<Synapse>;
		postSynapticSynapses = new Set<Synapse>;
		preSynapticFiring = 0;
    };

	virtual void addPostSynapticNeuron(Neuron *neuron) = 0;
	void addPostSynapticNeurons(Layer<AssociativeNeuron> *neurons);

	virtual void propegate(void) = 0;

	void addFiring(double weight)
    {
		preSynapticFiring += weight;
    };


	virtual void fire(void);

	virtual bool isRefractored(void) = 0;

	virtual void raise(Neuron *pre, Neuron* post){};
	virtual void lower(Neuron *pore, Neuron* post){};
  
	Synapse* getPostSynapticSynapse(Neuron *neuron);
      
	Set<Synapse> *postSynapticSynapses;
	Set<Synapse> *preSynapticSynapses;
	string *name;
protected:
	double preSynapticFiring;
};

class AssociativeNeuron : public Neuron
{
public:
	AssociativeNeuron(string n)
    {
		name = new string(n);
		logger = new AssociativeNeuronLogger(this);
		firingTimes = new FiringTimes();
		tauM = 4; tauS = 2; tauR = 2;
		uR = 0; uM = 0; uS = 0;
		threshold =.2;
		lowerFactor = 1.5;
		refractoriness = 0;
		deltaAbs = 2;
		activeThreshold = .001;
		relatedThreshold = 0;
    };
	~AssociativeNeuron(void)
    {
		delete logger;
    };

	void finalize(void)
    {
		logger->finalize();
    };
	void addPostSynapticNeuron(Neuron *neuron);
    

	void propegate(void);
 
	bool calculateThreshold(void) 
    {
		double epsilon = calculateEpsilon();
		double eta = calculateEta();
  
		if ( eta + epsilon >= threshold)
		{ 
			return(1);
		}
		else
		{ 
			return(0);
		}
    };

	double getPotential(void)
    {
		return (uR + uS + uM);
    }
	double calculateEpsilon(void)
    {
		calculateDecays();
		calculatePresynapticFiring();
		return(uM + uS);
    };

	double calculateEta(void)
    {
		uR = uR / exp (1 / tauR);
      
		if(!firingTimes->isEmpty() && 
		   ( (NOW - firingTimes->getLast()->getValue() ) <  deltaAbs))
		{
			refractoriness = 1;
		}
		else
		{
			refractoriness = 0;
		}
		return(uR);
    }; 
  
	void calculateDecays(void)
    {
		uM = uM / exp (1 / tauM);
		uS = uS / exp (1 / tauS);
    };

	void calculatePresynapticFiring(void)
    {
		uM += preSynapticFiring;
		uS -= preSynapticFiring;
  
		preSynapticFiring = 0;
    };

    
	void fire(void);
    
	void resetPotential(void)
    {
		uR = uR - ( lowerFactor * threshold);
    };

	bool isRefractored(void)
    {
		return(refractoriness);
    };

	double getRelatedness(AssociativeNeuron *neuron)
    {
		Item<Double> *thisRate;
		Item<Double> *neuronRate;
		double total;

		if (logger->rates != NULL && neuron->logger->rates != NULL)
		{
			thisRate = logger->rates->first;
			neuronRate = neuron->logger->rates->first;
	  
			double thisAverage = 1 * logger->rates->getAverage();
			double neuronAverage = neuron->logger->rates->getAverage();
	  
			while (thisRate && neuronRate)
			{
				total += (thisRate->item->getValue() - thisAverage) *
					(neuronRate->item->getValue() - neuronAverage);
	  
				thisRate = thisRate->next;
				neuronRate = neuronRate->next;
			}
		}
		return(total);
    };

	bool isActive(void)
    {
		if (logger->rates->getAverage() > activeThreshold)
		{
			return(1);
		}
      
		return(0);
    };
	
	bool relatedTo(AssociativeNeuron *postSynapticNeuron)
    {
		if (getRelatedness(postSynapticNeuron) > relatedThreshold)
		{
			return(1);
		}
		return(0);
    };

	void printStatistics(Set<Pattern> *patterns);

	AssociativeNeuronLogger* logger;

	double threshold;
private:

	double uM, uS, uR;
	double tauR, tauM, tauS;
	double lowerFactor;
	double relatedThreshold, activeThreshold;
	int deltaAbs;
	bool refractoriness;
	FiringTimes *firingTimes;

};

class QueuedFirings : public Queue<Int>
{
public:
	int checkForFiring(void)
    {
      
		if (getFirst() == NOW && getFirst() > -1 )
		{
			removeAndDelete();
			return(1);
		}
      
		return(0);
	};

	int getFirst(void)
    {
		if (first != NULL)
		{
	  
			return(first->item->getValue());
		}
		else
		{

			return(-1);
		}
	};
};

class Synapse
{
public:
	Synapse(Neuron *preN, Neuron *postN)
    {
		preSynapticNeuron = preN;
		postSynapticNeuron = postN;
		queuedFirings = new QueuedFirings();
		deltaAxon = 1;
    };
	void fire(void)
    {
		queuedFirings->add(new Int(NOW + deltaAxon));
    };

	virtual void propegate(void) = 0;
	virtual double getWeight(void) = 0;
	virtual void raise(double){};
	virtual void lower(double){};

	Neuron *preSynapticNeuron;
	Neuron *postSynapticNeuron;
 
	QueuedFirings *queuedFirings;
	int deltaAxon;
	double weight;
};

  

class HebbianSynapse : public Synapse
{
public:
HebbianSynapse(Neuron *preN, Neuron *postN):
    Synapse(preN, postN)
    {
		weight = 0;
		r = 0.02;
		tauR = 50;
    };

	void propegate(void)
    {
		calculateDecay();
      
		if (queuedFirings->checkForFiring() )
		{
			postSynapticNeuron->addFiring(getWeight());
			if (wR - r > -1)
				wR -= r;
			else
				wR = -1;
		} 
    };  

	double getWeight(void)
    {
        return(weight * (1 + wR));
    };

	void calculateDecay(void)
    {
		wR = wR / exp(1 / (double)tauR);
    };

	void raise(double diff)
	{
		weight += diff;
	};

	void lower(double diff)
	{
		weight -= diff;
	};
private:
	double r;
	double wR;
	int tauR;
};

class StaticSynapse : public Synapse
{
public:
StaticSynapse(Neuron *preN, Neuron *postN): 
    Synapse(preN, postN)
    {
		weight = 1;
    };

	double getWeight(void)
	{
		return(weight);
	};

	void propegate(void)
	{
	  
		if ( queuedFirings->checkForFiring() )
		{
			postSynapticNeuron->addFiring(getWeight());
		}
	};
	    
};



class AddressNeuron : public Neuron
{
public:
	AddressNeuron(void)
    {
		isActive = 0;
		firingChance = .5;
		refractorinessTime = 2;
    };

	void addPostSynapticNeuron(Neuron *neuron)
    {
		StaticSynapse *synapse = new StaticSynapse(this, neuron);
		postSynapticSynapses->add(synapse);
		neuron->preSynapticSynapses->add(synapse);
    };
  
	void propegate(void)
    {
		postSynapticSynapses->forallDo(&Synapse::propegate);
		if (isActive)
		{
			refractoriness--;
			if ( (double) rand() / (double) RAND_MAX < firingChance)
			{
				fire();
				refractoriness = refractorinessTime;
			}
		}
    };
      
	bool isRefractored(void)
    {
		if (refractoriness > 0)
		{
			return(1);
		}
		else
		{
			return(0);
		}
    };
  
	bool isActive;
private:
	double firingChance;
	int refractoriness;
	int refractorinessTime;
  
};


template<class LayerType> class Layer : public Set<LayerType>
{
public:
	void propegate(void)
	{
		Item<LayerType> *temp;
    
		for (temp = Set<LayerType>::first; temp; temp = temp->next)
		{
			temp->item->propegate();
		}
	};

	Layer<LayerType>* getRandomSubLayer(int subSetSize)
    {
		Item<LayerType> *neuron;
		Layer *newLayer = new Layer<LayerType>();
      
		int size = 0;
		int index;

		for (neuron = Set<LayerType>::first; neuron; neuron = neuron->next)
		{
			size++;
		}

		while (subSetSize > 0)
		{
			index = rand() % size;
			neuron = Set<LayerType>::first;
			while (index > 0)
			{
				index--;
				neuron = neuron->next;
			}
			if ( !(newLayer->in(neuron->item)))
			{
				newLayer->add(neuron->item);
				subSetSize--;
			}
		}
            
		return(newLayer);
    };

  
};



class AssociativeLayer : public Layer<AssociativeNeuron>
{
public:
	AssociativeLayer(AddressLayer *aL, int s, int pS, int aS)
	{
		addressLayer = aL;
		size = s;
		patternSize = pS;
		addressSize = aS;
		patterns = new Set<Pattern>();
		AssociativeNeuron *neuron;
		cout << "Creating Associative Neurons\n";
		while (s > 0)
		{
			char c[5];
			sprintf(c, "%d", s);
			string pre = "./log/Neuron ";
			add(new AssociativeNeuron( pre + c));
			s--;
		}
		connectNeurons();
    
		calculatePatterns();
     
	};

	~AssociativeLayer(void)
    {
		Item<AssociativeNeuron> *temp;

		for (temp = first; temp; temp = temp->next)
		{
			delete temp->item;
		}
    };
  
	void printStatistics(char* filename);
    
	void finalize(void)
    {
		Item<AssociativeNeuron> *temp;
      
		for (temp = first; temp; temp = temp->next)
		{
			temp->item->finalize();
		}
		//printStatistics("log/AssociativeLayer");
    };

	/*

	  Set< Set<AssociativeNeuron> >* getPatterns(void)
	  {
      Set< Set<AssociativeNeuron> > *patterns = 
	  new Set< Set<AssociativeNeuron> >;
      Set<AssociativeNeuron>* pattern;
      Item<AssociativeNeuron> *neuron;
      Item<AssociativeNeuron> *postSynaptic;

      cout << "gettting ActiveNeurons\n";
      Set<AssociativeNeuron> *activeNeurons = getActiveNeurons();
            

      for (neuron = activeNeurons->first; neuron; neuron = neuron->next)
      {
	  pattern = new Set<AssociativeNeuron>;
	  patterns->add(pattern);
	

	  pattern->add(neuron->item);
	    
	  for( postSynaptic = activeNeurons->first; postSynaptic; 
	  postSynaptic = postSynaptic->next)
	  {
	  if (neuron->item->relatedTo(postSynaptic->item))
	  {
	      
	  pattern->add(postSynaptic->item);
	  }
	  }
      }
      cout << "return\n";
      return(patterns);
	  };


	  Set<AssociativeNeuron>* getActiveNeurons(void)
	  {
      Set<AssociativeNeuron>* activeNeurons = new Set<AssociativeNeuron>;
      Item<AssociativeNeuron> *neuron;
      
      for(neuron = first; neuron; neuron = neuron->next)
	  {
	  if (neuron->item->isActive())
	  {
	  activeNeurons->add(neuron->item);
	  }
	  }
      return(activeNeurons);
	  };
	*/    
    Set< Pattern > *patterns;
private:      
      

	void connectNeurons(void)
    {
		Item<AssociativeNeuron> *preSynaptic;

		cout << "Connecting Neurons\n";
		for (preSynaptic = first; preSynaptic; preSynaptic = preSynaptic->next)
		{
			preSynaptic->item->addPostSynapticNeurons( this);
		}
		cout << "Neurons connected\n" ;
    };

	void calculatePatterns(void);
  
	AddressLayer *addressLayer;
	int size;
	int patternSize;
	int addressSize;
};

class Pattern : public Layer<AssociativeNeuron>
{
public:
	Pattern(AddressNeuron *address, int patternSize, 
			int addressSize, AssociativeLayer *layer)
    {
		size = patternSize;
		addressNeuron = address;

		if (addressNeuron->isActive)
		{
			isActive = 1;
		}
		first = layer->getRandomSubLayer(patternSize)->first;
		calculateHebbianWeights(layer);
		addressNeuron->addPostSynapticNeurons( this->
											   getRandomSubLayer(addressSize));
    };

	void calculateHebbianWeights(Layer<AssociativeNeuron> *layer)
    {
		Item<AssociativeNeuron> *preSynaptic;
		Item<AssociativeNeuron> *postSynaptic;
      
		for (preSynaptic = first; preSynaptic; 
			 preSynaptic = preSynaptic->next)
		{
			for (postSynaptic = layer->first; postSynaptic; 
				 postSynaptic = postSynaptic->next)
			{
				if (preSynaptic != postSynaptic)
				{
					if ( in(postSynaptic->item))
					{
						raise(preSynaptic->item, postSynaptic->item);
					}
					else
					{
						lower(preSynaptic->item, postSynaptic->item);
					}
				}
			}
		}
    };

    
	void raise(Neuron *preSynaptic, Neuron *postSynaptic)
	{
		if (preSynaptic != postSynaptic)
		{
			preSynaptic->getPostSynapticSynapse(postSynaptic)->raise(.15);
			postSynaptic->getPostSynapticSynapse(preSynaptic)->raise(.15);
		}
	};

	void lower(Neuron *preSynaptic, Neuron *postSynaptic)
	{
		if (preSynaptic != postSynaptic)
		{
			preSynaptic->getPostSynapticSynapse(postSynaptic)->lower(.05);
			postSynaptic->getPostSynapticSynapse(preSynaptic)->lower(.05);
		}
	};

	double averageRelation(AssociativeNeuron* related)
    {
		Item<AssociativeNeuron> *neuron;
		double total = 0;
		int nr = 0;
      
		for(neuron = first; neuron; neuron = neuron->next)
		{
			nr++;
			total += neuron->item->getRelatedness(related);
		}
		return(total / (double) nr);
    };

	string* getName(void)
    {
		return(addressNeuron->name);
    }

	bool isActive;
private:
	int size;
	AddressNeuron *addressNeuron;
};

class AddressLayer : public Layer<AddressNeuron>
{
public:
	AddressLayer(int nrP , int nrA ) 
	{
		size = nrP;
		Layer<AddressNeuron> *activePatterns;
		Item<AddressNeuron> *temp;

		while (nrP)
		{
			AddressNeuron *n = new AddressNeuron();
			char c[5];
			sprintf(c, "%d", nrP);

			n->name = new string(c);
			add(n);
			nrP--;
		}
		activePatterns = getRandomSubLayer(nrA);
    
		cout << "Activating Patter\n"; 
		for( temp = activePatterns->first; temp; temp = temp->next)
		{
			temp->item->isActive = 1;
			cout << "activating neuron\n";
		}

	};

	void printActivePatterns(void)
    {
		Item<AddressNeuron> *neuron;
		Item<Synapse> *postSynapse;
      
		for (neuron = first; neuron ; neuron = neuron->next)
		{
			if (neuron->item->isActive)
			{
				cout << "\nActive Pattern\n";
				for (postSynapse = neuron->item->postSynapticSynapses->first;
					 postSynapse; postSynapse = postSynapse->next)
				{
					cout << *(postSynapse->item->postSynapticNeuron->name)
						 <<  " ";
				}
				cout << "\n";
			}
		}
    };

	void finalize(void)
    {
		ofstream stream("log/AddressLayer", ios::trunc);
		Item<AddressNeuron> *neuron;
		Item<Synapse> *postSynapse;
      
		for (neuron = first; neuron ; neuron = neuron->next)
		{
			if (neuron->item->isActive)
			{
				stream << "Active Pattern:\n";
				for (postSynapse = neuron->item->postSynapticSynapses->first;
					 postSynapse; postSynapse = postSynapse->next)
				{
					stream << *(postSynapse->item->postSynapticNeuron->name)
						   <<  " ";
					;
				}
				stream << "\n";
			}
		}
    };
      
private:
	int size;

};

 

class AssociativeNetwork
{
public:
	AssociativeNetwork(int size, int nrOfPatterns, int activePatterns,
					   int patternsSize, int addressSize)
    {
		srand(time(NULL));
		cout << "Creating Address Layer\n";
		addressLayer = new AddressLayer(nrOfPatterns, activePatterns);
		cout << "Creating Associative Layer\n";
		associativeLayer = new AssociativeLayer(addressLayer, size, 
												patternsSize, addressSize);
		addressLayer->printActivePatterns();

    };
	~AssociativeNetwork()
    {
		delete addressLayer;
		delete associativeLayer;
	};
	void propegate(void)
    {
		addressLayer->propegate();
		associativeLayer->propegate();
      
		NOW++;
    };

	void propegate(int time)
	{
		while(time)
		{
			cout << "\r" << time;
			propegate();
			time--;
		  
		}
		addressLayer->finalize();
		associativeLayer->finalize();
      
		Item<AssociativeNeuron> *neuron;
		for (neuron = associativeLayer->first; neuron; neuron = neuron->next)
		{
			neuron->item->printStatistics(associativeLayer->patterns);
		}
	  
      
	};

      
private:
	AssociativeLayer *associativeLayer;
	AddressLayer *addressLayer;
};





Synapse* Neuron::getPostSynapticSynapse(Neuron *neuron)
{
	Item<Synapse> *temp;
  
	for (temp = postSynapticSynapses->first; temp; temp = temp->next)
    {
		if (temp->item->postSynapticNeuron == neuron)
		{
			return(temp->item);
		}
    }
	return(NULL);
};

void Neuron::fire(void)
{
	if (!isRefractored())
    {
		postSynapticSynapses->forallDo (&Synapse::fire);
    }
}


void AssociativeNeuron::propegate(void)
{
	logger->logPotential();
	logger->logFiring();
	postSynapticSynapses->forallDo(&Synapse::propegate);
	if ( calculateThreshold() )
    {
		fire();
    }

}

void AssociativeNeuron::fire(void)
{
	if (!isRefractored())
    {
		postSynapticSynapses->forallDo (&Synapse::fire);
		resetPotential();
		logger->logFiringTime();
    }
}

void AssociativeNeuron::addPostSynapticNeuron(Neuron *neuron)
{
	HebbianSynapse *synapse = new HebbianSynapse(this, neuron);
	postSynapticSynapses->add(synapse);
	neuron->preSynapticSynapses->add(synapse);
}

void Neuron::addPostSynapticNeurons(Layer<AssociativeNeuron> *neurons)
{
	Item<AssociativeNeuron> *temp;
  
	for (temp = neurons->first; temp; temp = temp->next)
    {
		if (temp->item != this)
			addPostSynapticNeuron(temp->item);
    }
}

void AssociativeNeuronLogger::finalize(void)
{
	rates = new FiringRateQueue(firingTimes, *(neuron->name), 10);
};


void AssociativeLayer::calculatePatterns(void)
{
	Item<AddressNeuron> *temp;
  
	for (temp = addressLayer->first; temp; temp = temp->next)
    {
		Pattern *pattern = new Pattern(temp->item, patternSize,
									   addressSize, this);
		patterns->add(pattern);
    }
}

void AssociativeNeuronLogger::logPotential(void)
{
	potentials->add(new Double(neuron->getPotential()));
};

void AssociativeNeuronLogger::logFiring(void)
{
	
	if (neuron->getPotential() > neuron->threshold)
	{
		firings->add(new Int(num));
	}
	else
	{
		firings->add(new Int(0));
	}

};


AssociativeNeuronLogger::AssociativeNeuronLogger(AssociativeNeuron* n)
{
	neuron = n;
   
	num = atoi(neuron->name->substr(13).c_str());
	firingTimes = new FiringTimesQueue(*(neuron->name));
	potentials = new PotentialsQueue(*(neuron->name));
	firings = new FiringQueue(*(neuron->name));  
};
  
FiringTimesQueue::~FiringTimesQueue(void)
{
	Item<Int> *time;

	for (time = first; time; time = time->next)
    {
		*stream << time->item->getValue() << "\n";
    }

	stream->close();
}


void AssociativeLayer::printStatistics(char* filename)
{
	Item<Pattern > *pattern;
	Item<AssociativeNeuron> *neuron;
	int index = 0;
  
	ofstream stream(filename, ios::trunc);

	stream << "Size: " << size << "\n";
 
	stream << "Retlated to: " << *(patterns->first->item->first->item->name) << "\n";

	for (pattern = patterns->first; pattern; pattern = pattern->next)
    {
		stream << "    Pattern: "<< " "<< index++;
		if (pattern->item->isActive)
		{
			stream << "(active)\n";
	
			for (neuron = pattern->item->first; neuron; neuron = neuron->next)
			{
				stream << *(neuron->item->name) << " average: "
					   << neuron->item->logger->rates->getAverage() << " rel:"
					   << neuron->item->getRelatedness(patterns->first->item->first->item);
			}
		}
		stream << "\n";
    }
};



void AssociativeNeuron::printStatistics(Set<Pattern> *patterns)
{

	Item<Pattern> *pattern;
	double relation;

	for (pattern = patterns->first; pattern; pattern = pattern->next)
    {
		if (pattern->item->isActive)
		{
			relation = pattern->item->averageRelation(this);

			if (pattern->item->in(this) )  
			{
				if (relation < 0) {
					//cout << "\n" << *name <<  " " << logger->rates->getAverage() << "\n\n";
					//cout << (*pattern->item->getName()) << " In: "
					//	 << relation << "\n";
				}
			}
	  
			if ( !pattern->item->in(this) && relation)	
			{
				if (relation > 0.0001)
				{
					//cout << "\n" << *name <<  " " << logger->rates->getAverage() << "\n\n";
					//cout << (*pattern->item->getName()) << " Out: "
					//	 << relation << "\n";
				}		
	      
			}
		} 
    }
};
