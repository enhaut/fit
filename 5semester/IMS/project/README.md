# Mass service system
## Assignment
Construct mass service system. Model wisely chosen (or hypothetical) logistical company (goods transport, 
goods delivery, Rohlik.cz etc., waste management, special transport of people - if exists ;)). 
Show bottlenecks in the system. Focus on logistics management strategies.


## Approach
## Introduction

The aim of this project is to build models representing terminals on the eastern border of the European Union and Ukraine, followed by their simulations. The experiments using these models aim to increase the capacity for unloading wheat from broad-gauge wagons. In the current situation, it is important to understand how to quickly increase the capacity of terminals on the eastern border of the European Union.

### Authors

The author of this project is Samuel Dobroň. The main sources of information were the official websites of terminal operators, public interviews with their representatives (cross-verified from multiple sources), and communication via email with responsible personnel of the terminals. Furthermore, this simulation model (1, slide 7) could not have been developed without the knowledge acquired in the course [Modeling and Simulation](https://www.fit.vut.cz/study/course/IMS/.cs).

## Input Data and Facts

As mentioned in [1] and based on our own examination of the railway map in [2], it was determined that there are six railway terminals available for wheat transshipment on the borders of Europe and Ukraine. The most important data defining their capacity include the container unloading/loading speed and the train arrival and departure speed at the pre-loading terminal. Additionally, the transportation speed of goods to the terminal is a contributing factor, although it is not a property of the terminal itself.

For wheat transportation, according to [3], bulk or universal 20-foot containers (also known as TEUs - Twenty Equipment Units) are used. As stated in [1] and [4], Ukraine has only 731 bulk containers suitable for wheat transportation and 24,000 universal containers. According to [5] and [6], a single container can hold 23-24 tons of wheat.

### Čierna nad Tisou [SK]

Čierna nad Tisou has a capacity of 200,000 bulk containers annually, as reported in [7]. The terminal is focused on bulk materials.

According to Kristína Janošová (janosova.kristina@zscargo.sk), the average time for train arrival at the terminal is 4 minutes, and the time for train departure is 5 minutes. The maximum length of such a train is 70 wagons. She also confirmed the correctness of the container unloading speed (including shifting and removing the wagon), mentioned as 2 minutes in [8].

## Záhony [HU]

The Záhony terminal is the newest in Europe and has the highest capacity, reaching up to 1,000,000 TEUs when utilizing all three available terminals with a length of 28 wagons. Each terminal has its own crane on separate tracks, eliminating the need to move the train. The cranes have capacities of 18, 24, and 24 tons. The terminal cannot handle bulk containers, so universal containers must be used. The container transfer speed, train arrival/departure times were obtained from Gabor Juszku (juszkugabor@zahony-port.hu). On average, all cranes can transfer a container in 84 seconds, train arrival takes 2 minutes, and train departure takes 3 minutes. Each of the three tracks can accommodate 29 wagons with containers.

## [PL] Medyka and Dorohursk

The Dorohursk terminal provides transshipment only for bulk materials, while the Medyka terminal does not provide bulk material transshipment. The annual TEU capacity for both terminals is not publicly available. Missing data was obtained from the terminal representative, wolka@tradetrans.pl. For the Medyka terminal, with an annual capacity of 280,000 TEUs, train arrival takes 5 minutes, container transshipment takes 4 minutes, and train departure takes 4 minutes. The maximum train length can be 39 wagons. The Dorohusk terminal, with a capacity of 100,000 TEUs per year, has a maximum train length of 30 wagons, 2 minutes for train arrival, 3 minutes for train departure, and 5 minutes for container transshipment.

## [RO] Halmeu and Dornesti

These terminals do not publicly disclose their annual capacity but provide information on the TEU transshipment speed and the useful lengths of their tracks. Both terminals state a value of 2 minutes per 1 TEU for transshipment [9] [10]. Train arrival and departure times are less relevant compared to container transshipment (e.g., for a train with 70 containers, unloading takes 140 minutes compared to train arrival of 4 minutes and departure of 5 minutes). For Dornesti, which can accommodate half the number of wagons (15) compared to the Záhony terminal, the times used were 1 minute for train arrival and 2 minutes for train departure. Halmeu has a slightly longer useful track length, with a capacity for 19 wagons, and the times chosen were 2 minutes for train arrival and 2 minutes for train departure. According to [11], the Dornesti terminal has two terminals with one shared crane, while the Halmeu terminal has one track and one crane [10].

## Validation

To partially verify the validity of this model, the `exp0` was conducted. This experiment simulated uninterrupted operations of the terminals for one year without any external influences, such as events that reduce the transport capacity of the tracks leading to the terminals or a shortage of wagons, locomotives, or personnel.

This model allowed for measuring the throughput of the modeled terminals and comparing it with real data provided by the operators. By comparing the model's output with real values, it was determined that the terminal models are valid.

What could not be validated is the railway network on the Ukrainian side since some information regarding the maximum speed of the tracks is not available on [3] or other sources. The missing data is replaced with the average speed at the beginning and end of the track where the speed information is not available. It is customary for track speeds on connecting sections to be the same or have a small difference. Therefore, this part of the model can also be considered valid.


## Models

The task of this project is to simulate the operation of a bulk handling system in terminals used for wheat transshipment at the border between Europe and Ukraine.

For this purpose, six terminal models were created to track the annual transshipment capacity and the process of this activity. Due to the high complexity of the real system (track capacity, multiple railway routes to the terminals, the problem of traveling salesman [^1], various failures, etc.), it is not possible to simulate the entire system completely.

This project focuses solely on the terminals themselves, specifically their annual wheat transshipment capacity and other factors that may indirectly affect the capacity. The following factors are neglected:
- Loading speed on the Ukrainian side
- Travel time from the place of origin to the terminal
- Various track restrictions (speed limits, track capacity)
- Speed of transferring transshipped wheat on the European side further into Europe
- Possibility of the terminal handling other goods besides wheat

A simplified fact is:
- The time required to unload a container
    - In reality, this time is not constant, but the model assumes a constant time due to the unavailability of more detailed data
    - It includes the time required for potential crane movement or other necessary equipment

### Model design
Abstract models of the queuing system were described using stochastic Petri nets.
[petri nets](koncepcia-modelov.pdf)

## Experiments

### Experiment 0
This experiment verifies the input data of the translation stations' model, which defines their annual throughput in TEUs (Twenty Equipment Units, a term used in logistics for a 20-foot shipping container). The input data includes:
- The maximum number of train wagons that a translation station can process, determined by the length of the useful track.
- The train approach time, which is the time required for the train to arrive at the translation station from the entrance/siding track.
- The container handling time.
- The train removal time from the translation station.

Throughout the experiment, all translation stations were fully utilized, indicating that the translation station itself was the bottleneck.

Results of Experiment 0 in TEUs:

| Translation Station | Model Throughput | Reported Throughput | Difference |
|---------------------|-----------------:|--------------------:|-----------:|
| Čierna n. Tisou [SK] |          199,920 |             200,000 |    -0.04% |
| Záhony [HU]         |        1,002,762 |           1,000,000 |    +0.27% |
| Medyka [PL]         |          280,800 |             280,000 |    +0.28% |
| Dorohursk [PL]      |          101,700 |             100,000 |    +1.7%  |
| Dornesti [RO]       |          245,280 |           Not stated |     --     |
| Halmeu [RO]         |          237,766 |           Not stated |     --     |

The results of this experiment demonstrate that the throughput of the translation station models is almost identical to the reported values provided by the operators, except for the Dornesti and Halmeu translation stations.

These stations only disclose the handling speed and the useful length of the track, which is sufficient for modeling. Comparing them to Čierna nad Tisou, where the TEU handling time is the same, we can conclude that the models of these translation stations are also valid.

### Experiment 1
The goal of this experiment is to identify the bottleneck in the Dornesti translation station.

From the simulation using the translation station model, it is evident that the shared crane between two tracks is the bottleneck:
| Device    | Average Crane Waiting Time during Train Unloading (min) | Average Unloading Time (min) |
|-----------|-------------------------------------------------------:|-------------------------------:|
| Track 1   |                                                 27.99 |                          58.99 |
| Track 2   |                                                 28.00 |                          58.99 |

By adding an additional crane so that each track has its own crane, the train unloading time is significantly reduced:
| Device    | Average Crane Waiting Time during Train Unloading (min) | Average Unloading Time (min) |
|-----------|-------------------------------------------------------:|-------------------------------:|
| Track 1   |                                                  0.00 |                          31.00 |
| Track 2   |                                                  0.00 |                          31.00 |

The results of this experiment demonstrate that the shared crane is the bottleneck in the Dornesti translation station. By adding an extra crane, each track will have its own crane, leading to a reduction in train unloading time from 59 minutes to 31 minutes, representing a 47% reduction. This reduction corresponds to an increase in throughput from 245,280 TEUs to **492,735** TEUs.

### Experiment 2
The purpose of this experiment is to confirm or refute the assumption that the Záhony translation station has a higher capacity when using differently loaded universal containers. As mentioned in the [Záhony translation station section](#zahony), the station has three tracks, each equipped with a separate crane with load capacities of 24, 24, and 18 tons.

An universal container can hold between 23 and 24 tons of wheat. This experiment considers using 24-ton containers to maximize efficiency.

| Number of Cranes Used | Annual Capacity (TEUs) | Annual Wheat Handling Capacity (tons) |
|-----------------------|----------------------:|--------------------------------------:|
| 2                     |               668,508 |                            16,044,192 |

The experiment uses 18-ton containers for every third train and 24-ton containers for the remaining trains.

| Crane          | Translated TEUs | Translated Tons |
|----------------|----------------:|----------------:|
| 24-ton Crane 1 |         334,316 |       8,023,584 |
| 24-ton Crane 2 |         334,207 |       8,020,968 |
| 18-ton Crane   |         334,260 |       6,016,680 |
| Total          |       1,002,783 |      22,061,232 |

The results of the experiment show that it is more efficient to send partially loaded universal containers, weighing 18 tons of wheat instead of their full capacity of 24 tons. This allows the translation station to utilize the third crane with a maximum load capacity of 18 tons, increasing the overall capacity of the station from 16,044,192 tons of wheat (when containers are fully loaded at 24 tons) to **22,061,232 tons**.


## Conclusion
Based on the `Experiment 0`, it has been demonstrated that the translation station models are valid, as the simulation results closely correspond to real-world values.

In the next two experiments, improvement proposals were tested with positive outcomes.

In Experiment 1, the focus was on identifying the bottleneck of the Dornesti translation station, which was found to be a shared crane between two handling tracks. By adding an additional crane with the same parameters to the shared crane, the train handling time was reduced by 47%, resulting in a 100% increase in the capacity of the translation station.

Experiment 2 successfully confirmed that even using containers that are not fully loaded to their maximum capacity of 24 tons of wheat but rather partially loaded with 18 tons can increase the wheat handling capacity at the translation station. By loading every third train with 18-ton containers, the station can utilize a crane with a lower maximum load capacity, resulting in a 37% increase in the overall capacity of the translation station.

These experiments demonstrate potential improvements and optimizations for the translation stations, allowing for more efficient operations and increased handling capacity.




## Requirements

- C
- SIMLIB library - https://www.fit.vutbr.cz/~peringer/SIMLIB/



---

[1] https://www.intermodal-terminals.eu/database/terminal/view/id/424   
[2] https://unicom-tranzit.ro/en/facilities/#terminale   
[3] https://www.openrailwaymap.org/?style=maxspeed&amp;lat=48.73083222613515&amp;lon=24.14794921875&amp;zoom=8   
[4] https://eastwestil.com/en/terminal-2/   
[5] https://www.ect.kz/com_grain_en.html   
[6] https://www.ofimagazine.com/news/ukraines-grain-rail-transport-picks-up-but-remains-just-over-half-of-potential-capacity   
[7] https://www.hellenicshippingnews.com/containerized-shipment-of-grains-pushed-in-shift-from-bulk-cargo/   
[8] https://www.fastmarkets.com/insights/three-reasons-ukraines-railway-grain-movements-are-still-only-reaching-55-of-potential   
[9] https://www.efeedlink.com/contents/04-11-2012/575eaaae-10d4-4cf2-a14a-976347bac9cd-a002.html   
[10] https://youtu.be/HyXZcxr7l3I   
[11] https://blog.sme.sk/martinvozar1/ekonomika/nove-zeleznicne-prekladisko-pri-kosiciach-alebo-zbytocna-investicia-kosickej-zupy   
