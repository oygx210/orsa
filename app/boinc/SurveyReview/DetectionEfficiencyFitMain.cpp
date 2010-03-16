#include <orsa/statistic.h>
#include "skycoverage.h"
#include "eta.h"

class EfficiencyStatistics : public orsa::WeightedStatistic<double> {
public:
  EfficiencyStatistics(const double & aux) : 
    orsa::WeightedStatistic<double>(),
    center(aux) { }
public:
  const double center; // V or U
  orsa::Cache<double> fit; // eta_V or eta_U
};

// for sorting
class EfficiencyStatistics_ptr_cmp {
public:
  bool operator() (EfficiencyStatistics * lhs,
		   EfficiencyStatistics * rhs) const {
    return (lhs->center < rhs->center);
  }
};

int main(int argc, char ** argv) {
  
  orsa::Debug::instance()->initTimer();
  
  if (argc != 2) {
    printf("Usage: %s <allEta-file>\n",argv[0]);
    exit(0);
  }
  
  ORSA_DEBUG("process ID: %i",getpid());
  
  const std::string basename = SkyCoverage::basename(argv[1]);
  
  std::vector<EfficiencyData> etaData;
  {
    FILE * fp = fopen(argv[1],"r");
    if (!fp) {
      ORSA_DEBUG("cannot open file [%s]",argv[1]);
      exit(0);
    }
    char line[1024];
    double H, V, apparentVelocity;
    char id[1024];
    int observed;
    while (fgets(line,1024,fp)) {
      sscanf(line,"%lf %s %lf %lf %i",
	     &H,
	     id,
	     &V,
	     &apparentVelocity,
	     &observed);
      // convert
      apparentVelocity = orsa::FromUnits(apparentVelocity*orsa::arcsecToRad(),orsa::Unit::HOUR,-1);
      //
      EfficiencyData ed;
      ed.H=H;
      if (atoi(id) != 0) {
	ed.number = atoi(id);
      } else {
	ed.designation = id;
      }
      ed.V=V;
      ed.apparentVelocity=apparentVelocity;
      ed.observed = (observed==1);
      //
      etaData.push_back(ed);
    }
    fclose(fp);
  }
  
  const double V0=17.0;
  const double U0=orsa::FromUnits(100.0*orsa::arcsecToRad(),orsa::Unit::HOUR,-1); // arcsec/hour
  EfficiencyMultifit::DataStorage data;
  {
    char filename[1024];
    sprintf(filename,"%s.eta.dat",basename.c_str());
    FILE * fp_eta = fopen(filename,"w"); 
    
    double V=V0;
    const double dV=0.2;
    while (V<=24.0) {
      double apparentVelocity=orsa::FromUnits(0.01*orsa::arcsecToRad(),orsa::Unit::HOUR,-1);
      const double apparentVelocityFactor=1.2;
      while (apparentVelocity<orsa::FromUnits(300.0*orsa::arcsecToRad(),orsa::Unit::HOUR,-1)) {
	
	unsigned int Nobs=0,Ntot=0;
	for (unsigned int k=0; k<etaData.size(); ++k) {
	  if ((etaData[k].V.getRef()>=V) && 
	      (etaData[k].V.getRef()<V+dV) && 
	      (etaData[k].apparentVelocity.getRef()>=apparentVelocity) &&
	      (etaData[k].apparentVelocity.getRef()<apparentVelocityFactor*apparentVelocity)) {
	    ++Ntot;
	    if (etaData[k].observed.getRef()) {
	      ++Nobs;
	    }
	  }
	}
	
	// write point only if Ntot != 0
	if (Ntot!=0) {
	  const double      eta = (double)Nobs/(double)Ntot;
	  const double sigmaEta = (double)(sqrt(Nobs+1))/(double)(Ntot); // Poisson counting statistics; using Nobs+1 instead of Nobs to have positive sigma even when Nobs=0
	  
	  fprintf(fp_eta,
		  "%.6f %.6f %.6f %.6f %.6f %.6f %7i %7i\n",
		  V+0.5*dV,
		  dV,
		  orsa::FromUnits(apparentVelocity*0.5*(1.0+apparentVelocityFactor)*orsa::radToArcsec(),orsa::Unit::HOUR),
		  apparentVelocityFactor,
		  eta,
		  sigmaEta,
		  Nobs,
		  Ntot);
	  
	  
	  EfficiencyMultifit::DataElement el;
	  //
	  el.V=V;
	  el.U=apparentVelocity;
	  el.eta=eta;
	  el.sigmaEta=sigmaEta;
	  //
	  data.push_back(el);
	}
	
	apparentVelocity *= apparentVelocityFactor;
      }
      
      V += dV;
    }
    
    fclose(fp_eta);
  }    
  
  osg::ref_ptr<EfficiencyMultifit> etaFit= new EfficiencyMultifit;
  const bool success = etaFit->fit(data,V0,U0);
  if (!success) { 
    ORSA_DEBUG("problems??");
  }
  osg::ref_ptr<const orsa::MultifitParameters> parFinal = etaFit->getMultifitParameters();
  // save final parameters
  const double  eta0_V = parFinal->get("eta0_V");
  const double     c_V = parFinal->get("c_V");
  const double V_limit = parFinal->get("V_limit");
  const double     w_V = parFinal->get("w_V");
  //
  const double U_limit_slow = parFinal->get("U_limit_slow");
  const double     w_U_slow = parFinal->get("w_U_slow");
  // const double U_limit_fast = parFinal->get("U_limit_fast");
  // const double     w_U_fast = parFinal->get("w_U_fast");
#warning keep vars in sync with fitting code
  
  {
    // output for testing
    typedef std::list< osg::ref_ptr<EfficiencyStatistics> > AllStat;
    AllStat stat_V, stat_U;  
    //
    for (unsigned int k=0; k<data.size(); ++k) {
      
      osg::ref_ptr<EfficiencyStatistics> sV;
      {
	AllStat::const_iterator it = stat_V.begin();
	while (it != stat_V.end()) {
	  if ((*it).get() != 0) {
	    if ((*it)->center == data[k].V.getRef()) {
	      sV = (*it).get();
	    }   
	  }
	  ++it;
	}
	if (sV.get() == 0) {
	  sV = new EfficiencyStatistics(data[k].V.getRef());
	  stat_V.push_back(sV);
	}
      }
      
      osg::ref_ptr<EfficiencyStatistics> sU;
      {
	AllStat::const_iterator it = stat_U.begin();
	while (it != stat_U.end()) {
	  if ((*it).get() != 0) {
	    if ((*it)->center == data[k].U.getRef()) {
	      sU = (*it).get();
	    }   
	  }
	  ++it;
	}
	if (sU.get() == 0) {
	  sU = new EfficiencyStatistics(data[k].U.getRef());
	  stat_U.push_back(sU);
	}
      }
      
      const double eta_V = SkyCoverage::eta_V(data[k].V.getRef(),
					      V_limit,
					      eta0_V,
					      c_V,
					      V0,
					      w_V);

      const double eta_U = SkyCoverage::eta_U(data[k].U.getRef(),
					      U_limit_slow,
					      w_U_slow,
					      U0);
      
      if (data[k].sigmaEta.getRef() > 0) {
	if (eta_U != 0) {
	  sV->insert(data[k].eta.getRef()/eta_U,
		     orsa::square(eta_U/data[k].sigmaEta.getRef()));
	}
	if (eta_V != 0) {
	  sU->insert(data[k].eta.getRef()/eta_V,
		     orsa::square(eta_V/data[k].sigmaEta.getRef()));
	}
      }
      
      if (!sV->fit.isSet()) sV->fit = eta_V;
      if (!sU->fit.isSet()) sU->fit = eta_U;
    }
    
    // sort lists
    stat_V.sort(EfficiencyStatistics_ptr_cmp());
    stat_U.sort(EfficiencyStatistics_ptr_cmp());
    
    {
      FILE * fpv = fopen("v.fit.dat","w");
      AllStat::const_iterator it = stat_V.begin();
      while (it != stat_V.end()) {
	if (!(*it)->fit.isSet()) {
	  ORSA_DEBUG("problems: fit value not set");
	  ++it;
	  continue;
	}
	fprintf(fpv,"%g %g %g %g\n",
		(*it)->center,
		(*it)->fit.getRef(),
		(*it)->average(),
		(*it)->averageError());
	++it;
      }
      fclose(fpv);
    }
    
    {
      FILE * fpu = fopen("u.fit.dat","w");
      AllStat::const_iterator it = stat_U.begin();
      while (it != stat_U.end()) {
	if (!(*it)->fit.isSet()) {
	  ORSA_DEBUG("problems: fit value not set");
	  ++it;
	  continue;
	}
	fprintf(fpu,"%g %g %g %g\n",
		orsa::FromUnits((*it)->center*orsa::radToArcsec(),orsa::Unit::HOUR), // arcsec/hour
		(*it)->fit.getRef(),
		(*it)->average(),
		(*it)->averageError());
	++it;
      }
      fclose(fpu);
    }
    
  }
  
  exit(0);
}