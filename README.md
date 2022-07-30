# distRS 

A distributed computing framework for MODIS satellite imagery high-level products processing

![architecture](architecture.png "distRS architecture")

# Description[^1]

With the various types of actual ET models being developed in the last 20 years, it becomes
necessary to inter-compare methods. Most of already published ETa models comparisons
address few number of models, and small to medium areas (Chemin et al., 2010; Gao & Long,
2008; García et al., 2007; Suleiman et al., 2008; Timmermans et al., 2007). With the large amount
of remote sensing data covering the Earth, and the daily information available for the past ten
years (i.e. Aqua/Terra-MODIS) for each pixel location, it becomes paramount to have a more
complete comparison, in space and time.

To address this new experimental requirement, a distributed computing framework was
designed, and created. The design architecture was built from original satellite datasets
to various levels of processing until reaching the requirement of various ETa models input
dataset. Each input product is computed once and reused in all ETa models requiring such
input. This permits standardization of inputs as much as possible to zero-in variations of
models to the models internals/specificities.

[^1]: http://www.intechopen.com/books/evapotranspiration-remote-sensing-and-modeling/a-distributed-benchmarking-framework-for-actual-et-models

